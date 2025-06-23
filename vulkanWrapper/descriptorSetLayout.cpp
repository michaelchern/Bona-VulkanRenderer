
#include "descriptorSetLayout.h"

namespace LearnVulkan::Wrapper
{

    DescriptorSetLayout::DescriptorSetLayout(const Device::Ptr &device)
    {
        mDevice = device;
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        if (mLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(mDevice->getDevice(), mLayout, nullptr);
        }
    }

    // 构建描述符集布局
    // 参数 params - Uniform参数列表，定义绑定点配置
    void DescriptorSetLayout::build(const std::vector<UniformParameter::Ptr>& params)
    {
        // 1. 存储参数（可选，视后续需求）
        mParams = params;

        // 2. 清理现有布局（如果已存在）
        if (mLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(mDevice->getDevice(), mLayout, nullptr);
        }

        // 3. 转换参数为Vulkan绑定结构
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};

        for (const auto& param : mParams)
        {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.descriptorType  = param->mDescriptorType;  // 类型 (UBO/Sampler等)
            layoutBinding.binding         = param->mBinding;         // 绑定点索引 (对应shader中的binding=X)
            layoutBinding.stageFlags      = param->mStage;           // 着色器可见阶段 (VERTEX/FRAGMENT等)
            layoutBinding.descriptorCount = param->mCount;           // 描述符数量 (支持数组)
            // 注：pImmutableSamplers高级功能暂未使用

            layoutBindings.push_back(layoutBinding);
        }

        // 4. 创建Vulkan描述符集布局
        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());         // 绑定点数量
        createInfo.pBindings    = layoutBindings.data();                                // 绑定点配置数组

        // 调用Vulkan API创建布局
        if (vkCreateDescriptorSetLayout(mDevice->getDevice(), &createInfo, nullptr, &mLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create descriptor set layout");
        }
    }

}