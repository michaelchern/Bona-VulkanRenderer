#pragma once

#include "base.h"
#include "device.h"
#include "commandPool.h"

namespace LearnVulkan::Wrapper
{
    /*
    * 分析：如果我们需要做一张被用于纹理采样的图片，那么我们首先
    * 需要从 undefinedLayout 变换成为 TransferDst，然后在数据拷贝
    * 完毕之后，再转换称为ShaderReadOnly
    */

    /*
    * 分析：纹理图片处理流程通常需要分阶段转换图像布局：
    * 1. 初始布局: VK_IMAGE_LAYOUT_UNDEFINED
    * 2. 转换为传输目标布局: VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL (用于数据拷贝)
    * 3. 数据拷贝完成后转换为着色器只读布局: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL (用于采样)
    */

    class Image
    {
    public:
        using Ptr = std::shared_ptr<Image>;
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
                                           aspectFlags
            );
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

        //此处属于便捷写法，封装性比较好，但是可以独立作为一个工具函数
        //写到Tool的类里面

        // 图像布局转换方法（便捷功能）
        // 参数说明：
        //   newLayout：目标布局
        //   srcStageMask：源管线阶段（屏障起始点）
        //   dstStageMask：目标管线阶段（屏障结束点）
        //   subresrouceRange：图像子资源范围（层级、层次等）
        //   commandPool：命令池（用于创建一次性的命令缓冲区）
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

    private:
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    private:
        size_t         mWidth{ 0 };
        size_t         mHeight{ 0 };
        Device::Ptr    mDevice{ nullptr };
        VkImage        mImage{ VK_NULL_HANDLE };
        VkDeviceMemory mImageMemory{ VK_NULL_HANDLE };
        VkImageView    mImageView{ VK_NULL_HANDLE };
        VkImageLayout  mLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
    };
}
