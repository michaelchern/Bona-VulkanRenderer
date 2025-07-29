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

        // Image 类构造函数：用于创建 Vulkan 图像（VkImage）、分配内存并创建图像视图（VkImageView）
        // 参数说明：
        // - device: Vulkan 设备指针（管理图形硬件的逻辑设备）
        // - width: 图像宽度（像素）
        // - height: 图像高度（像素）
        // - format: 图像像素格式（如 VK_FORMAT_R8G8B8A8_UNORM）
        // - imageType: 图像类型（通常为 VK_IMAGE_TYPE_2D 表示二维图像）
        // - tiling: 图像内存排列方式（VK_IMAGE_TILING_OPTIMAL：硬件优化排列；VK_IMAGE_TILING_LINEAR：线性排列）
        // - usage: 图像用途标志（如 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 颜色附件、VK_IMAGE_USAGE_SAMPLED_BIT 采样纹理）
        // - sample: 多重采样等级（如 VK_SAMPLE_COUNT_1_BIT 单采样，VK_SAMPLE_COUNT_4_BIT 四重采样）
        // - properties: 内存属性（如 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT 设备本地内存、VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 主机可见内存）
        // - aspectFlags: 图像方面标志（如 VK_IMAGE_ASPECT_COLOR_BIT 颜色通道、VK_IMAGE_ASPECT_DEPTH_BIT 深度通道）
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

        /**
         * @brief 转换图像的内存布局（Image Layout Transition）
         *
         * Vulkan中图像（Image）的内存布局需要显式转换才能在不同操作间正确使用（如从传输布局转为着色器读取布局）。
         * 此函数通过提交内存屏障（Memory Barrier）实现布局转换，确保不同阶段（如传输、着色器）对图像的访问同步。
         *
         * @param newLayout 目标图像布局（如VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL）
         * @param srcStageMask 源阶段掩码：指定执行旧布局操作的管线阶段（如VK_PIPELINE_STAGE_TRANSFER_BIT）
         * @param dstStageMask 目标阶段掩码：指定执行新布局操作的管线阶段（如VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT）
         * @param subresrouceRange 需要转换布局的子资源范围（如特定mipmap层、面）
         * @param commandPool 用于创建命令缓冲的命令池
         */
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
        VkFormat       mFormat{ VK_FORMAT_UNDEFINED };
        VkImageLayout  mLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
    };
}
