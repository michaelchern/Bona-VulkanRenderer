#include "texture.h"

// 启用stb_image实现
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"  // 使用第三方图像加载库

namespace LearnVulkan
{

    /**
    * @brief 构造函数（从文件加载纹理）
    *
    * 完整创建流程：
    *  1. 加载图像文件
    *  2. 创建Vulkan图像资源
    *  3. 转换图像布局
    *  4. 填充图像数据
    *  5. 转换为着色器可读布局
    *  6. 创建采样器
    *  7. 设置描述符信息
    */
    Texture::Texture(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr & commandPool, const std::string& imageFilePath)
    {
        mDevice = device;  // 保存逻辑设备引用

        // [步骤1] 加载图像文件
        int texWidth, texHeight, texSize, texChannles;

        // 强制加载为RGBA格式（4通道）即使原始图像是RGB
        stbi_uc* pixels = stbi_load(
            imageFilePath.c_str(),  // 文件路径
            &texWidth,              // 输出宽度
            &texHeight,             // 输出高度
            &texChannles,           // 输出通道数（不使用）
            STBI_rgb_alpha          // 强制4通道
        );

        // 检查加载结果
        if (!pixels)
        {
            throw std::runtime_error("Error: failed to read image data");
        }

        // 计算像素数据大小（宽×高×4字节）
        texSize = texWidth * texHeight * 4;

        // [步骤2] 创建Vulkan图像资源
        mImage = Wrapper::Image::create(
            mDevice,                              // Vulkan逻辑设备
            texWidth,                             // 图像宽度
            texHeight,                            // 图像高度
            VK_FORMAT_R8G8B8A8_SRGB,              // SRGB色彩空间格式
            VK_IMAGE_TYPE_2D,                     // 2D纹理
            VK_IMAGE_TILING_OPTIMAL,              // 最佳内存布局（通常线性布局）
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,  // 用途：传输目标 + 着色器采样
            VK_SAMPLE_COUNT_1_BIT,                // 多重采样关闭
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,  // 设备本地内存
            VK_IMAGE_ASPECT_COLOR_BIT             // 颜色图像
        );

        // [步骤3] 准备布局转换（转为传输目标布局）
        VkImageSubresourceRange region{};
        region.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;  // 操作颜色通道
        region.baseArrayLayer = 0;  // 起始数组层
        region.layerCount     = 1;  // 单层纹理
        region.baseMipLevel   = 0;  // 起始mip级别
        region.levelCount     = 1;  // 单mip级别（无mipmap）

        // 转换布局：初始布局 → 传输目标布局
        mImage->setImageLayout(
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,  // 目标布局
            VK_PIPELINE_STAGE_TRANSFER_BIT,        // 源管线阶段
            VK_PIPELINE_STAGE_TRANSFER_BIT,        // 目标管线阶段
            region,                                // 操作区域
            commandPool                            // 用于执行转换的命令池
        );

        // [步骤4] 填充图像数据（通过暂存缓冲区）
        mImage->fillImageData(
            texSize,        // 数据大小
            (void*)pixels,  // 像素数据指针
            commandPool     // 命令池（用于传输命令）
        );

        // [步骤5] 转换布局为着色器只读布局
        mImage->setImageLayout(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,  // 最终布局
            VK_PIPELINE_STAGE_TRANSFER_BIT,            // 源阶段：传输完成
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,     // 目标阶段：片段着色器
            region,                                    // 操作区域
            commandPool                                // 命令池
        );

        // 释放CPU端像素数据（重要！避免内存泄漏）
        stbi_image_free(pixels);

        // [步骤6] 创建关联的采样器
        mSampler = Wrapper::Sampler::create(mDevice);

        // [步骤7] 配置描述符图像信息
        mImageInfo.imageLayout = mImage->getLayout();     // 图像当前布局
        mImageInfo.imageView   = mImage->getImageView();  // 图像视图句柄
        mImageInfo.sampler     = mSampler->getSampler();  // 采样器句柄
    }

    /// 析构函数（当前为空，依赖智能指针自动清理资源）
    Texture::~Texture() {}
}