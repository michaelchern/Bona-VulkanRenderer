#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    // Vulkan采样器封装类（负责配置纹理采样参数）
    class Sampler
    {
    public:
        using Ptr = std::shared_ptr<Sampler>;
        static Ptr create(const Device::Ptr& device) { return std::make_shared<Sampler>(device); }

        Sampler(const Device::Ptr &device);

        ~Sampler();

        [[nodiscard]] auto getSampler() const { return mSampler; }

    private:
        Device::Ptr mDevice{ nullptr };
        VkSampler   mSampler{ VK_NULL_HANDLE };
    };
}
