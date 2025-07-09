#pragma once

#include "base.h"
#include "device.h"
#include "description.h"

namespace LearnVulkan::Wrapper
{
    class DescriptorSetLayout
    {
    public:
        using Ptr = std::shared_ptr<DescriptorSetLayout>;
        static Ptr create(const Device::Ptr& device) { return std::make_shared<DescriptorSetLayout>(device); }

        DescriptorSetLayout(const Device::Ptr &device);

        ~DescriptorSetLayout();

        void build(const std::vector<UniformParameter::Ptr>& params);

        [[nodiscard]] auto getLayout() const { return mLayout; }

    private:
        VkDescriptorSetLayout mLayout{ VK_NULL_HANDLE };
        Device::Ptr           mDevice{ nullptr };

        std::vector<UniformParameter::Ptr> mParams{};
    };
}