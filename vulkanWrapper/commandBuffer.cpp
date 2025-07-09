
#include "commandBuffer.h"

namespace LearnVulkan::Wrapper
{
    /**
     * @brief 构造函数 - 分配命令缓冲区
     *
     * @param device 关联的逻辑设备
     * @param commandPool 分配缓冲区的命令池
     * @param asSecondary 是否创建二级命令缓冲区（默认主缓冲区）
     */
    CommandBuffer::CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary)
    {
        mDevice      = device;       // 存储设备对象
        mCommandPool = commandPool;  // 存储命令池对象

        // 配置命令缓冲区分配信息
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandBufferCount = 1;                               // 分配1个命令缓冲区
        allocInfo.commandPool        = mCommandPool->getCommandPool();  // 指定命令池

        // 设置命令缓冲级别：主/二级
        allocInfo.level = asSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY
                                      : VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        // 调用Vulkan API分配命令缓冲区
        if (vkAllocateCommandBuffers(mDevice->getDevice(), &allocInfo, &mCommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: Falied to create commandBuffer!");
        }
    }

    /**
     * @brief 析构函数 - 释放命令缓冲区
     *
     * 注意：命令缓冲区由命令池统一管理，池销毁时自动释放所有缓冲区。
     * 此处显式释放可提前回收资源。
     */
    CommandBuffer::~CommandBuffer()
    {
        if (mCommandBuffer != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(mDevice->getDevice(), mCommandPool->getCommandPool(), 1, &mCommandBuffer);
        }
    }

    /**
     * @brief 开始命令记录
     *
     * @param flag 使用标志：
     *    - VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT:      一次性使用
     *    - VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: 二级缓冲区
     *    - VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT:     可重复提交
     * @param inheritance 二级缓冲继承信息（仅当flag包含RENDER_PASS_CONTINUE时有效）
     */
    void CommandBuffer::begin(VkCommandBufferUsageFlags flag, const VkCommandBufferInheritanceInfo& inheritance)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags            = flag;          // 设置使用标志
        beginInfo.pInheritanceInfo = &inheritance;  // 继承信息（二级缓冲区）

        if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Error:failed to begin commandBuffer!");
        }
    }

    /**
     * @brief 开始渲染通道
     *
     * @param renderPassBeginInfo 渲染通道起始信息
     * @param subPassContents 子通道内容类型：
     *   - VK_SUBPASS_CONTENTS_INLINE:                    主缓冲区直接记录命令
     *   - VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: 使用二级缓冲区
     */
    void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo, const VkSubpassContents& subPassContents)
    {
        vkCmdBeginRenderPass(mCommandBuffer, &renderPassBeginInfo, subPassContents);
    }

    /// @brief 绑定图形管线
    void CommandBuffer::bindGraphicPipeline(const VkPipeline& pipeline)
    {
        vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    /// @brief 绑定顶点缓冲区数组（多个缓冲区）
    void CommandBuffer::bindVertexBuffer(const std::vector<VkBuffer>& buffers)
    {
        std::vector<VkDeviceSize> offsets(buffers.size(), 0);  // 所有缓冲区偏移设为0

        vkCmdBindVertexBuffers(mCommandBuffer, 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
    }

    /// @brief 绑定索引缓冲区（默认为32位索引）
    void CommandBuffer::bindIndexBuffer(const VkBuffer& buffer)
    {
        vkCmdBindIndexBuffer(mCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    /// @brief 绑定描述符集（着色器资源）
    void CommandBuffer::bindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet &descriptorSet)
    {
        vkCmdBindDescriptorSets(mCommandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout,
                                0,               // 第一个描述符集
                                1,               // 描述符集数量
                                &descriptorSet,  
                                0,               // 动态偏移量数量
                                nullptr);        // 动态偏移数组
    }

    /// @brief 绘制非索引几何体（直接绘制顶点）
    void CommandBuffer::draw(size_t vertexCount)
    {
        vkCmdDraw(mCommandBuffer,
                  vertexCount,  // 顶点数量
                  1,            // 实例数量
                  0,            // 首个顶点索引
                  0);           // 首个实例索引
    }

    /// @brief 索引绘制（使用绑定的索引缓冲区）
    void CommandBuffer::drawIndex(size_t indexCount)
    {
        vkCmdDrawIndexed(mCommandBuffer,
                         indexCount,  // 索引数量
                         1,           // 实例数量
                         0,           // 首个索引偏移
                         0,           // 顶点偏移
                         0);          // 首个实例索引
    }

    /// @brief 结束渲染通道
    void CommandBuffer::endRenderPass()
    {
        vkCmdEndRenderPass(mCommandBuffer);
    }

    /// @brief 结束命令记录
    void CommandBuffer::end()
    {
        if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Error:failed to end Command Buffer!");
        }
    }

    /**
     * @brief 缓冲区间复制（数据传输）
     *
     * @param srcBuffer 源缓冲区句柄
     * @param dstBuffer 目标缓冲区句柄
     * @param copyInfoCount 复制操作数量
     * @param copyInfos 复制区域描述数组
     */
    void CommandBuffer::copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyInfos)
    {
        vkCmdCopyBuffer(mCommandBuffer, srcBuffer, dstBuffer, copyInfoCount, copyInfos.data());
    }

    /**
     * @brief 从缓冲区复制数据到图像
     *
     * @param srcBuffer 源缓冲区
     * @param dstImage 目标图像
     * @param dstImageLayout 目标图像当前布局（必须是可写入状态）
     * @param width 图像宽度（像素）
     * @param height 图像高度（像素）
     */
    void CommandBuffer::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height)
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;                                         // 缓冲区起始偏移
        region.bufferRowLength = 0;                                      // 0表示数据紧密排列
        region.bufferImageHeight = 0;                                    // 0表示无额外行高

        // 配置图像子资源
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // 颜色通道
        region.imageSubresource.mipLevel = 0;                            // 基础mipmap层级
        region.imageSubresource.baseArrayLayer = 0;                      // 基础数组层
        region.imageSubresource.layerCount = 1;                          // 仅1层

        // 设置图像区域
        region.imageOffset = {0, 0, 0};                                  // 起始坐标(x,y,z)
        region.imageExtent = {width, height, 1};                         // 复制区域大小

        vkCmdCopyBufferToImage(
            mCommandBuffer,
            srcBuffer,
            dstImage,
            dstImageLayout,  // 图像当前布局
            1,               // 区域数量
            &region          // 复制区域描述
        );
    }

    /**
     * @brief 图像布局转换（内存屏障）
     *
     * @param imageMemoryBarrier 图像内存屏障配置
     * @param srcStageMask 源管线阶段
     * @param dstStageMask 目标管线阶段
     */
    void CommandBuffer::transferImageLayout(const VkImageMemoryBarrier &imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
    {
        vkCmdPipelineBarrier(
            mCommandBuffer, 
            srcStageMask,           // 源管线阶段
            dstStageMask,           // 目标管线阶段
            0,                      // 依赖标志
            0, nullptr,             // 内存屏障数组
            0, nullptr,             // 缓冲区内存屏障数组
            1, &imageMemoryBarrier  // 图像内存屏障数组
        );
    }

    /**
     * @brief 同步提交命令到队列（阻塞CPU直到执行完成）
     *
     * @param queue 目标队列
     * @param fence 可选栅栏对象（用于异步操作）
     */
    void CommandBuffer::submitSync(VkQueue queue, VkFence fence)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;                 // 提交1个命令缓冲区
        submitInfo.pCommandBuffers = &mCommandBuffer;      // 指定命令缓冲区

        // 提交到队列
        vkQueueSubmit(queue, 1, &submitInfo, fence);

        // 阻塞等待队列空闲（实现CPU-GPU同步）
        vkQueueWaitIdle(queue);
    }
}