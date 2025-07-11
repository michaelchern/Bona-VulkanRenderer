
#include "image.h"
#include "commandBuffer.h"
#include "buffer.h"

namespace LearnVulkan::Wrapper
{
    Image::Image(const Device::Ptr &device,
                 const int& width,
                 const int& height,
                 const VkFormat& format,
                 const VkImageType& imageType,
                 const VkImageTiling& tiling,
                 const VkImageUsageFlags& usage,
                 const VkSampleCountFlagBits& sample,
                 const VkMemoryPropertyFlags& properties,
                 const VkImageAspectFlags& aspectFlags)
    {
        mDevice = device;
        mLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        mWidth = width;
        mHeight = height;

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.extent.width  = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth  = 1;
        imageCreateInfo.format        = format;
        imageCreateInfo.imageType     = imageType;
        imageCreateInfo.tiling        = tiling;
        imageCreateInfo.usage         = usage;
        imageCreateInfo.samples       = sample;
        imageCreateInfo.mipLevels     = 1;
        imageCreateInfo.arrayLayers   = 1;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(mDevice->getDevice(), &imageCreateInfo, nullptr, &mImage) != VK_SUCCESS)
        {
            throw std::runtime_error("Error:failed to create image!");
        }

        VkMemoryRequirements memReq{};
        vkGetImageMemoryRequirements(mDevice->getDevice(), mImage, &memReq);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;

        allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

        if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mImageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to allocate memory!");
        }

        vkBindImageMemory(mDevice->getDevice(), mImage, mImageMemory, 0);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        imageViewCreateInfo.viewType                        = imageType == VK_IMAGE_TYPE_2D ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;
        imageViewCreateInfo.format                          = format;
        imageViewCreateInfo.image                           = mImage;

        imageViewCreateInfo.subresourceRange.aspectMask     = aspectFlags;
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.levelCount     = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(mDevice->getDevice(), &imageViewCreateInfo, nullptr, &mImageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create image view!");
        }
    }

    Image::~Image()
    {
        if (mImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(mDevice->getDevice(), mImageView, nullptr);
        }

        if (mImageMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(mDevice->getDevice(), mImageMemory, nullptr);
        }

        if (mImage != VK_NULL_HANDLE)
        {
            vkDestroyImage(mDevice->getDevice(), mImage, nullptr);
        }
    } 

    uint32_t Image::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(mDevice->getPhysicalDevice(), &memProps);

        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties))
            {
                return i;
            }
        }

        throw std::runtime_error("Error: cannot find the property memory type!");
    }

    void Image::setImageLayout(VkImageLayout newLayout,
                               VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask,
                               VkImageSubresourceRange subresrouceRange,
                               const CommandPool::Ptr& commandPool)
    {
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout           = mLayout;
        imageMemoryBarrier.newLayout           = newLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image               = mImage;
        imageMemoryBarrier.subresourceRange    = subresrouceRange;

        switch (mLayout)
        {
            // 如果是无定义layout，说明图片刚被创建，上方一定没有任何操作，所以上方是一个虚拟的依赖
            // 所以不关心上一个阶段的任何操作
        case VK_IMAGE_LAYOUT_UNDEFINED:
            imageMemoryBarrier.srcAccessMask = 0;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // 当前是传输目标：确保之前的写入完成
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        // 可扩展其他布局情况...
        default:
            break;
        }

        // 目标访问掩码：基于目标布局确定后续需要什么操作权限
        switch (newLayout)
        {
            // 如果目标是，将图片转换成为一83个复制操作的目标图片/内存，那么被阻塞的操作一定是写入操作
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // 目标是传输目标：准备接收写入操作
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
            //如果目标是，将图片转换成为一个适合被作为纹理的格式，那么被阻塞的操作一定是，读取
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // 目标是着色器只读：后续需要读权限
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            break;
        }

        // 更新当前布局状态
        mLayout = newLayout;

        auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
        commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        commandBuffer->transferImageLayout(imageMemoryBarrier, srcStageMask, dstStageMask);
        commandBuffer->end();

        commandBuffer->submitSync(mDevice->getGraphicQueue());
    }

    void Image::fillImageData(size_t size,
                              void* pData,
                              const CommandPool::Ptr& commandPool)
    {
        assert(pData);
        assert(size);

        auto stageBuffer = Buffer::createStageBuffer(mDevice, size, pData);

        auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
        commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
        commandBuffer->copyBufferToImage(stageBuffer->getBuffer(),
                                         mImage,
                                         mLayout,
                                         mWidth,
                                         mHeight);

        commandBuffer->end();

        commandBuffer->submitSync(mDevice->getGraphicQueue());
    }
}
