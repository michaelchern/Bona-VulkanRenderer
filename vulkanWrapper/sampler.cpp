#include "sampler.h"

namespace LearnVulkan::Wrapper
{
    // 构造函数：创建Vulkan采样器对象
    Sampler::Sampler(const Device::Ptr &device)
    {
        mDevice = device;  // 保存逻辑设备引用，用于后续资源管理

        // 配置采样器参数
        VkSamplerCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;  // 结构体类型标识

        // 纹理过滤设置
        createInfo.magFilter = VK_FILTER_LINEAR;  // 放大时使用线性过滤（平滑过渡）
        createInfo.minFilter = VK_FILTER_LINEAR;  // 缩小时使用线性过滤

        // 纹理寻址模式
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;  // U坐标超出[0,1]时重复纹理
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;  // V坐标超出时重复
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;  // W坐标超出时重复

        // 各向异性过滤（提高倾斜表面的纹理质量）
        createInfo.anisotropyEnable = VK_TRUE;  // 启用各向异性过滤
        createInfo.maxAnisotropy    = 16;       // 最大过滤采样数（16是当前硬件的常用上限值）

        // 边界颜色（当使用CLAMP_TO_BORDER模式时）
        createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;  // 不透明的黑色边界

        // 我们是否使用非归一化的uvw坐标体系0-1
        // 坐标系统：使用归一化坐标（推荐）
        createInfo.unnormalizedCoordinates = VK_FALSE;  // 使用[0,1]标准化UV坐标

        // 采样得到的样本值需要跟一个值进行比较，比较的方式也可以设置，比较通过了测试，才采样，否则不采用
        // 比较操作（用于深度纹理或阴影贴图）
        createInfo.compareEnable = VK_FALSE;              // 禁用比较测试
        createInfo.compareOp     = VK_COMPARE_OP_ALWAYS;  // 默认始终通过比较

        // Mipmap相关设置
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;  // 线性插值选择mip级别
        createInfo.mipLodBias = 0.0f;                           // Mip级别偏移量（无偏移）
        createInfo.minLod     = 0.0f;                           // 最小mip级别
        createInfo.maxLod     = 0.0f;                           // 最大mip级别（0表示只使用基础级别）

        // 注意：实际应用中maxLod通常设为VK_LOD_CLAMP_NONE以使用所有可用mip级别

        // 创建Vulkan采样器对象
        if (vkCreateSampler(mDevice->getDevice(), &createInfo, nullptr, &mSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create sampler");
        }
    }

    // 析构函数：销毁Vulkan采样器资源
    Sampler::~Sampler()
    {
        if(mSampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(mDevice->getDevice(), mSampler, nullptr);
        }
    }
}