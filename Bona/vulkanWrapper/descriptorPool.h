#pragma once

#include "base.h"
#include "device.h"
#include "description.h"

namespace LearnVulkan::Wrapper
{
    class DescriptorPool
    {
    public:
        using Ptr = std::shared_ptr<DescriptorPool>;
        static Ptr create(const Device::Ptr& device) { return std::make_shared<DescriptorPool>(device); }

        DescriptorPool(const Device::Ptr &device);

        ~DescriptorPool();

        void build(std::vector<UniformParameter::Ptr>& params, const int &frameCount);

        [[nodiscard]] auto getPool() const { return mPool; }

    private:
        VkDescriptorPool mPool{ VK_NULL_HANDLE };
        Device::Ptr      mDevice{ nullptr };
    };
}
