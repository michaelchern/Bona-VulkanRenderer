#include "sampler.h"

namespace LearnVulkan::Wrapper
{
    Sampler::Sampler(const Device::Ptr &device)
    {
        mDevice = device;

        VkSamplerCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.minFilter = VK_FILTER_LINEAR;

        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        createInfo.anisotropyEnable = VK_TRUE;
        createInfo.maxAnisotropy    = 16;

        createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        createInfo.unnormalizedCoordinates = VK_FALSE;

        createInfo.compareEnable = VK_FALSE;
        createInfo.compareOp     = VK_COMPARE_OP_ALWAYS;

        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.mipLodBias = 0.0f;
        createInfo.minLod     = 0.0f;
        createInfo.maxLod     = 0.0f;

        if (vkCreateSampler(mDevice->getDevice(), &createInfo, nullptr, &mSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create sampler!");
        }
    }

    Sampler::~Sampler()
    {
        if(mSampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(mDevice->getDevice(), mSampler, nullptr);
        }
    }
}
