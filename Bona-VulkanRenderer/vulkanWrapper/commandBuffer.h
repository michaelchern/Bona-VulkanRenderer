#pragma once

#include "base.h"
#include "commandPool.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    class CommandBuffer
    {
    public:

        using Ptr = std::shared_ptr<CommandBuffer>;

        static Ptr create(const Device::Ptr &device, const CommandPool::Ptr &commandPool, bool asSecondary = false)
        { 
            return std::make_shared<CommandBuffer>(device, commandPool, asSecondary);
        }

        CommandBuffer(const Device::Ptr &device, const CommandPool::Ptr &commandPool, bool asSecondary = false);

        ~CommandBuffer();

        void begin(VkCommandBufferUsageFlags flag = 0, const VkCommandBufferInheritanceInfo &inheritance = {});

        void beginRenderPass(const VkRenderPassBeginInfo &renderPassBeginInfo, const VkSubpassContents &subPassContents = VK_SUBPASS_CONTENTS_INLINE);

        void bindGraphicPipeline(const VkPipeline &pipeline);

        void bindVertexBuffer(const std::vector<VkBuffer> &buffers);

        void bindIndexBuffer(const VkBuffer &buffer);

        void bindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet &descriptorSet);

        void draw(size_t vertexCount);

        void drawIndex(size_t indexCount);

        void endRenderPass();

        void end();

        void copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy> &copyInfos);

        // 功能：将缓冲区（Buffer）中的数据复制到图像（Image）的指定区域
        // 参数说明：
        //   srcBuffer: 源缓冲区句柄（数据来源）
        //   dstImage: 目标图像句柄（数据目标）
        //   dstImageLayout: 目标图像的布局（需为支持传输写入的布局，如VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL）
        //   width: 要复制的图像区域宽度（像素）
        //   height: 要复制的图像区域高度（像素）
        void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height);

        void transferImageLayout(const VkImageMemoryBarrier &imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

        void submitSync(VkQueue queue, VkFence fence = VK_NULL_HANDLE);

        [[nodiscard]] auto getCommandBuffer() const { return mCommandBuffer; }

    private:
        VkCommandBuffer  mCommandBuffer{ VK_NULL_HANDLE };  
        Device::Ptr      mDevice{ nullptr };                
        CommandPool::Ptr mCommandPool{ nullptr };           
    };
}