
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

    void DescriptorSetLayout::build(const std::vector<UniformParameter::Ptr>& params)
    {
        mParams = params;

        if (mLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(mDevice->getDevice(), mLayout, nullptr);
        }

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};

        for (const auto& param : mParams)
        {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.descriptorType  = param->mDescriptorType;
            layoutBinding.binding         = param->mBinding;
            layoutBinding.stageFlags      = param->mStage;
            layoutBinding.descriptorCount = param->mCount;
            // 注：pImmutableSamplers高级功能暂未使用

            layoutBindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        createInfo.pBindings    = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(mDevice->getDevice(), &createInfo, nullptr, &mLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: Failed to create descriptor set layout!");
        }
    }
}
