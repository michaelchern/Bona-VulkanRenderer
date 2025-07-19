#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

namespace LearnVulkan
{

    Texture::Texture(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr & commandPool, const std::string& imageFilePath)
    {
        mDevice = device;

        // -------------------- 步骤1：加载图像文件数据（CPU 内存） --------------------
        int texWidth, texHeight, texSize, texChannles;

        // 使用 stb_image 库加载图像文件（跨平台轻量级图像加载库）
        // 参数说明：
        //   imageFilePath.c_str(): 图像文件路径
        //   &texWidth/&texHeight: 输出图像宽高（像素）
        //   &texChannles: 输出原始通道数（如 RGB=3，RGBA=4）
        //   STBI_rgb_alpha: 强制转换为 RGBA 格式（4 通道，忽略原始通道数）
        stbi_uc* pixels = stbi_load(imageFilePath.c_str(),
                                    &texWidth,
                                    &texHeight,
                                    &texChannles,
                                    STBI_rgb_alpha);

        if (!pixels)  // 加载失败处理
        {
            throw std::runtime_error("Error: failed to read image data!");
        }

        texSize = texWidth * texHeight * 4;  // 计算图像总字节数（RGBA 每个像素 4 字节）

        // -------------------- 步骤2：创建 Vulkan 图像对象（GPU 内存） --------------------
        // 创建一个 2D 纹理图像，用于存储 GPU 可访问的纹理数据
        // 参数说明（关键参数）：
        //   mDevice: Vulkan 设备
        //   texWidth/texHeight: 图像宽高（像素）
        //   VK_FORMAT_R8G8B8A8_SRGB: 图像格式（RGBA 8 位，sRGB 色彩空间）
        //   VK_IMAGE_TYPE_2D: 二维图像（非 1D/3D/立方体贴图）
        //   VK_IMAGE_TILING_OPTIMAL: 最优平铺方式（Vulkan 自动优化内存布局）
        //   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT:
        //     图像用途
        //       - TRANSFER_DST_BIT: 允许作为传输操作的目标（接收缓冲区数据）
        //       - SAMPLED_BIT: 允许被采样器采样（用于着色器读取）
        //   VK_SAMPLE_COUNT_1_BIT: 单采样（无多重采样）
        //   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: 内存属性（GPU 本地内存，高性能）
        //   VK_IMAGE_ASPECT_COLOR_BIT: 图像子资源方面（仅颜色通道）
        mImage = Wrapper::Image::create(mDevice,
                                        texWidth,
                                        texHeight,
                                        VK_FORMAT_R8G8B8A8_SRGB,
                                        VK_IMAGE_TYPE_2D,
                                        VK_IMAGE_TILING_OPTIMAL,
                                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                        VK_SAMPLE_COUNT_1_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        VK_IMAGE_ASPECT_COLOR_BIT);


        VkImageSubresourceRange region{};
        region.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.baseArrayLayer = 0;
        region.layerCount     = 1;
        region.baseMipLevel   = 0;
        region.levelCount     = 1;

        mImage->setImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT,
                               region,
                               commandPool);


        mImage->fillImageData(texSize,
                       (void*)pixels,
                              commandPool);

        mImage->setImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               region,
                               commandPool);

        stbi_image_free(pixels);

        mSampler = Wrapper::Sampler::create(mDevice);

        mImageInfo.imageLayout = mImage->getLayout();
        mImageInfo.imageView   = mImage->getImageView();
        mImageInfo.sampler     = mSampler->getSampler();
    }

    Texture::~Texture() {}
}
