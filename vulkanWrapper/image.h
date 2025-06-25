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
        static Ptr create(
            const Device::Ptr& device,
            const int& width,
            const int& height,
            const VkFormat& format,
            const VkImageType& imageType,
            const VkImageTiling& tiling,
            const VkImageUsageFlags& usage,
            const VkSampleCountFlagBits& sample,
            const VkMemoryPropertyFlags& properties,  // 内存属性要求 (e.g., DEVICE_LOCAL)
            const VkImageAspectFlags& aspectFlags)    // 图像视图的观察方向 (e.g., COLOR)
        { 
            return std::make_shared<Image>(
                device,
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

        // 构造函数：创建Vulkan图像资源
        Image(
            const Device::Ptr &device,                // Vulkan逻辑设备包装器
            const int& width,                         // 图像宽度（像素）
            const int& height,                        // 图像高度（像素）
            const VkFormat &format,                   // 像素格式 (e.g., VK_FORMAT_R8G8B8A8_SRGB)
            const VkImageType &imageType,             // 图像类型 (e.g., VK_IMAGE_TYPE_2D)
            const VkImageTiling &tiling,              // 内存排布方式 (LINEAR/OPTIMAL)
            const VkImageUsageFlags &usage,           // 用途标志位 (e.g., TRANSFER_DST | SAMPLED)
            const VkSampleCountFlagBits &sample,      // 多重采样数
            const VkMemoryPropertyFlags &properties,  // 内存属性 (e.g., DEVICE_LOCAL)
            const VkImageAspectFlags &aspectFlags     // 图像视图的观察方向 (e.g., COLOR)
        );

        ~Image();  // 析构函数：释放Vulkan资源

        //此处属于便捷写法，封装性比较好，但是可以独立作为一个工具函数
        //写到Tool的类里面

        // 图像布局转换方法（便捷功能）
        // 参数说明：
        //   newLayout：目标布局
        //   srcStageMask：源管线阶段（屏障起始点）
        //   dstStageMask：目标管线阶段（屏障结束点）
        //   subresrouceRange：图像子资源范围（层级、层次等）
        //   commandPool：命令池（用于创建一次性的命令缓冲区）
        void setImageLayout(
            VkImageLayout newLayout,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            VkImageSubresourceRange subresrouceRange,
            const CommandPool::Ptr & commandPool
        );

        // 填充图像数据（主机到设备的内存传输）
        // 参数说明：
        //   size：数据字节数
        //   pData：主机端数据指针
        //   commandPool：命令池（用于创建传输命令）
        void fillImageData(size_t size, void* pData, const CommandPool::Ptr &commandPool);

        // --- Getter 方法（均带[[nodiscard]]属性提示返回值不可忽略）---
        [[nodiscard]] auto getImage() const { return mImage; }          // 获取VkImage句柄
        [[nodiscard]] auto getLayout() const { return mLayout; }        // 获取当前图像布局
        [[nodiscard]] auto getWidth() const { return mWidth; }          // 获取图像宽度
        [[nodiscard]] auto getHeight() const { return mHeight; }        // 获取图像高度
        [[nodiscard]] auto getImageView() const { return mImageView; }  // 获取图像视图句柄

    private:
        // 查找合适的内存类型索引（辅助函数）
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    private:
        size_t              mWidth{ 0 };                         // 图像宽度（像素）
        size_t              mHeight{ 0 };                        // 图像高度（像素）
        Device::Ptr         mDevice{ nullptr };                  // Vulkan逻辑设备包装器的智能指针
        VkImage             mImage{ VK_NULL_HANDLE };            // Vulkan图像对象句柄
        VkDeviceMemory      mImageMemory{ VK_NULL_HANDLE };      // 图像内存句柄
        VkImageView         mImageView{ VK_NULL_HANDLE };        // 图像视图句柄
        VkImageLayout       mLayout{VK_IMAGE_LAYOUT_UNDEFINED};  // 当前图像布局状态
    };
}