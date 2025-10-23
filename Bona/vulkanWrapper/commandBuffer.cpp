
#include "commandBuffer.h"

namespace LearnVulkan::Wrapper
{
    CommandBuffer::CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary)
    {
        mDevice      = device;
        mCommandPool = commandPool;

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandBufferCount = 1;                               
        allocInfo.commandPool        = mCommandPool->getCommandPool();  

        allocInfo.level = asSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY
                                      : VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(mDevice->getDevice(), &allocInfo, &mCommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: falied to create commandBuffer!");
        }
    }

    CommandBuffer::~CommandBuffer()
    {
        if (mCommandBuffer != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(mDevice->getDevice(), mCommandPool->getCommandPool(), 1, &mCommandBuffer);
        }
    }

    void CommandBuffer::begin(VkCommandBufferUsageFlags flag, const VkCommandBufferInheritanceInfo &inheritance)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags            = flag;
        beginInfo.pInheritanceInfo = &inheritance;

        if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to begin commandBuffer!");
        }
    }

    void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo &renderPassBeginInfo, const VkSubpassContents &subPassContents)
    {
        vkCmdBeginRenderPass(mCommandBuffer, &renderPassBeginInfo, subPassContents);
    }

    void CommandBuffer::bindGraphicPipeline(const VkPipeline &pipeline)
    {
        vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    void CommandBuffer::bindVertexBuffer(const std::vector<VkBuffer> &buffers)
    {
        std::vector<VkDeviceSize> offsets(buffers.size(), 0);

        vkCmdBindVertexBuffers(mCommandBuffer, 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
    }

    void CommandBuffer::bindIndexBuffer(const VkBuffer& buffer)
    {
        vkCmdBindIndexBuffer(mCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
    }

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

    void CommandBuffer::draw(size_t vertexCount)
    {
        vkCmdDraw(mCommandBuffer,
                  vertexCount,  
                  1,            
                  0,            
                  0);           
    }

    void CommandBuffer::drawIndex(size_t indexCount)
    {
        vkCmdDrawIndexed(mCommandBuffer,
                         indexCount,  // 索引数量
                         1,           // 实例数量
                         0,           // 首个索引偏移
                         0,           // 顶点偏移
                         0);          // 首个实例索引
    }

    void CommandBuffer::endRenderPass()
    {
        vkCmdEndRenderPass(mCommandBuffer);
    }

    void CommandBuffer::end()
    {
        if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to end Command Buffer!");
        }
    }

    void CommandBuffer::copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyInfos)
    {
        vkCmdCopyBuffer(mCommandBuffer, srcBuffer, dstBuffer, copyInfoCount, copyInfos.data());
    }

    void CommandBuffer::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height)
    {
        // 定义缓冲区到图像的复制区域描述结构体（Vulkan API核心结构）
        VkBufferImageCopy region{};

        // 缓冲区数据起始偏移量（从缓冲区的第0字节开始复制）
        region.bufferOffset = 0;

        // 缓冲区数据的"行长度"（单位：字节）：
        // 0表示缓冲区中的数据是**紧密排列**的（无额外行间距），Vulkan会自动根据图像格式计算实际行宽
        // 若图像是压缩格式（如BC格式），需手动计算并设置此值以保证对齐
        region.bufferRowLength = 0;

        // 缓冲区数据的"图像高度"（单位：像素）：
        // 0表示缓冲区中的数据是**单层连续存储**的（无额外层间间距），适用于非数组图像或仅复制第一层的情况
        region.bufferImageHeight = 0;

        // 配置要复制的图像子资源（Subresource）信息
        // 子资源用于定位图像中的具体部分（如Mipmap层级、数组层、颜色/深度通道等）
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // 仅复制图像的颜色通道（忽略深度/模板等通道）
        region.imageSubresource.mipLevel = 0;                            // 复制图像的基础Mipmap层级（最清晰的层级）
        region.imageSubresource.baseArrayLayer = 0;                      // 复制图像的起始数组层（适用于立方体贴图或数组图像）
        region.imageSubresource.layerCount = 1;                          // 仅复制1个数组层（若图像是数组图像）

        // 配置图像内的复制区域（相对于图像原点的偏移与尺寸）
        region.imageOffset = {0, 0, 0};                                  // 复制区域的起始坐标（x=0,y=0,z=0，Vulkan中Y轴默认向下）
        region.imageExtent = {width, height, 1};                         // 复制区域的尺寸（宽度×高度×深度=1，适用于二维图像）

        // 执行实际的复制命令（向当前命令缓冲区添加复制操作）
        // 参数说明：
        //   mCommandBuffer: 当前正在录制的命令缓冲区
        //   srcBuffer: 源缓冲区句柄
        //   dstImage: 目标图像句柄
        //   dstImageLayout: 目标图像的当前布局（必须与复制操作兼容）
        //   1: 要复制的子资源数量（此处仅1个子资源）
        //   &region: 描述复制区域的指针（包含偏移、尺寸等信息）
        vkCmdCopyBufferToImage(mCommandBuffer,
                               srcBuffer,
                               dstImage,
                               dstImageLayout,  
                               1,               
                               &region);
    }

    void CommandBuffer::transferImageLayout(const VkImageMemoryBarrier &imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
    {
        vkCmdPipelineBarrier(mCommandBuffer,
                             srcStageMask,         
                             dstStageMask,        
                             0,                  
                             0,
                             nullptr,             
                             0,
                             nullptr,             
                             1,
                             &imageMemoryBarrier);
    }

    /**
     * @brief 同步提交命令缓冲区到指定队列，并等待队列执行完成
     *
     * 该函数负责将当前命令缓冲区提交到Vulkan队列执行，并阻塞当前线程直至队列中所有操作完成。
     * 主要用于需要严格同步的场景（如确保命令执行完毕后再进行后续操作）。
     *
     * @param queue 目标Vulkan队列，命令将提交到此队列执行
     * @param fence 可选的同步栅栏（Vulkan对象），用于外部同步通知（提交完成后会触发此栅栏的信号）
     *              注：本函数内部会额外等待队列空闲，因此即使不使用此fence，也能保证命令执行完成
     */
    void CommandBuffer::submitSync(VkQueue queue, VkFence fence)
    {
        // 初始化提交信息结构体（Vulkan要求所有结构体必须显式设置sType字段）
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // 配置提交参数：
        submitInfo.commandBufferCount = 1;             // 本次提交包含1个命令缓冲区
        submitInfo.pCommandBuffers = &mCommandBuffer;  // 指向待提交的命令缓冲区（类成员变量）

        // 提交命令缓冲区到目标队列执行
        // 参数说明：
        // - queue: 目标队列
        // - 1: 提交的批次数量（对应commandBufferCount）
        // - &submitInfo: 提交批次的信息（此处仅1个批次）
        // - fence: 提交完成后会触发此栅栏的信号（外部可通过vkWaitForFences等待此栅栏）
        vkQueueSubmit(queue, 1, &submitInfo, fence);

        // 阻塞当前线程，等待目标队列完全空闲（所有已提交的操作执行完毕）
        // 注意：此操作会强制同步，确保队列中无未完成的命令，适用于需要严格同步的场景
        // 缺点：可能影响性能（阻塞线程），仅在必要时使用
        vkQueueWaitIdle(queue);
    }
}
