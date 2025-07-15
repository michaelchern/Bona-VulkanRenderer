#pragma once

#include "base.h"
#include "device.h"
#include "commandPool.h"

namespace LearnVulkan::Wrapper
{
    class Image
    {
    public:
        using Ptr = std::shared_ptr<Image>;

        static Image::Ptr createDepthImage(const Device::Ptr& device,
                                           const int& width,
                                           const int& height);

    public:
        
        static Ptr create(const Device::Ptr& device,
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
            return std::make_shared<Image>(device,
                                           width,
                                           height,
                                           format,
                                           imageType,
                                           tiling,
                                           usage,
                                           sample,
                                           properties,
                                           aspectFlags);
        }

        Image(const Device::Ptr &device,
              const int& width,
              const int& height,
              const VkFormat &format,
              const VkImageType &imageType,
              const VkImageTiling &tiling,
              const VkImageUsageFlags &usage,
              const VkSampleCountFlagBits &sample,
              const VkMemoryPropertyFlags &properties,
              const VkImageAspectFlags &aspectFlags);

        ~Image();

        void setImageLayout(VkImageLayout newLayout,
                            VkPipelineStageFlags srcStageMask,
                            VkPipelineStageFlags dstStageMask,
                            VkImageSubresourceRange subresrouceRange,
                            const CommandPool::Ptr & commandPool);

        void fillImageData(size_t size, void* pData, const CommandPool::Ptr &commandPool);

        [[nodiscard]] auto getImage()     const { return mImage; }
        [[nodiscard]] auto getLayout()    const { return mLayout; }
        [[nodiscard]] auto getWidth()     const { return mWidth; }
        [[nodiscard]] auto getHeight()    const { return mHeight; }
        [[nodiscard]] auto getImageView() const { return mImageView; }

    public:

        static VkFormat findDepthFormat(const Device::Ptr& device);

        static VkFormat findSupportedFormat(const Device::Ptr& device,
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features);

        bool hasStencilComponent(VkFormat format) const;

    private:
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        

    private:
        size_t         mWidth{ 0 };
        size_t         mHeight{ 0 };
        Device::Ptr    mDevice{ nullptr };
        VkImage        mImage{ VK_NULL_HANDLE };
        VkDeviceMemory mImageMemory{ VK_NULL_HANDLE };
        VkImageView    mImageView{ VK_NULL_HANDLE };
        VkFormat	   mFormat{ VK_FORMAT_UNDEFINED };
        VkImageLayout  mLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
    };
}
