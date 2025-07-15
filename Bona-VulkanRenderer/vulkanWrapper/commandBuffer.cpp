
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

    void CommandBuffer::submitSync(VkQueue queue, VkFence fence)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mCommandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, fence);

        vkQueueWaitIdle(queue);
    }
}