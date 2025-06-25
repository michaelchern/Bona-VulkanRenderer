#pragma once

#include "../vulkanWrapper/base.h"
#include "../vulkanWrapper/image.h"
#include "../vulkanWrapper/sampler.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/commandPool.h"

namespace LearnVulkan
{
    /**
    * @class Texture
    * @brief Vulkan纹理资源管理器（整合图像和采样器资源）
    *
    * 封装纹理的完整生命周期管理，包括：
    *  - 从文件加载图像数据
    *  - 创建设备本地图像资源
    *  - 设置采样器参数
    *  - 生成描述符所需的核心数据结构
    */
    class Texture
    {
    public:
        using Ptr = std::shared_ptr<Texture>;
        static Ptr create(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr &commandPool, const std::string& imageFilePath)
        {
            return std::make_shared<Texture>(device, commandPool, imageFilePath);
        }

        /**
        * @brief 构造函数（加载图像文件并创建Vulkan资源）
        *
        * 实现流程：
        *  1. 加载图像文件到内存
        *  2. 创建暂存缓冲区
        *  3. 创建设备本地图像
        *  4. 复制数据到设备内存
        *  5. 生成图像视图
        *  6. 创建采样器
        *  7. 初始化描述符信息
        */
        Texture(const Wrapper::Device::Ptr &device, const Wrapper::CommandPool::Ptr& commandPool, const std::string &imageFilePath);

        /// 析构函数：自动清理所有Vulkan资源
        ~Texture();

        /// 获取封装的图像对象（包含图像内容和视图）
        [[nodiscard]] auto getImage() const { return mImage; }
        
        /// 获取关联的纹理采样器
        [[nodiscard]] auto getSampler() const { return mSampler; }

        /**
        * @brief 获取描述符所需的图像信息结构
        *
        * 结构体内容：
        *   VkDescriptorImageInfo
        *   {
        *     .sampler     = 采样器句柄,
        *     .imageView   = 图像视图句柄,
        *     .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        *   }
        */
        [[nodiscard]] VkDescriptorImageInfo& getImageInfo() { return mImageInfo; }

    private:
        Wrapper::Device::Ptr  mDevice{ nullptr };   ///< 关联的Vulkan逻辑设备
        Wrapper::Image::Ptr   mImage{ nullptr };    ///< 图像资源管理器（图像+视图）
        Wrapper::Sampler::Ptr mSampler{ nullptr };  ///< 纹理采样器对象
        VkDescriptorImageInfo mImageInfo{};         ///< 描述符使用的图像信息
    };
}