#pragma once

#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/description.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/commandPool.h"
#include "vulkanWrapper/base.h"

using namespace LearnVulkan;

class UniformManager
{
public:
    using Ptr = std::shared_ptr<UniformManager>;
    static Ptr create() { return std::make_shared<UniformManager>(); }

    UniformManager();

    ~UniformManager();

    void init(const Wrapper::Device::Ptr &device, const Wrapper::CommandPool::Ptr &commandPool, int frameCount);

    void update(const VPMatrices &vpMatrices, const ObjectUniform &objectUniform, const int& frameCount);

    [[nodiscard]] auto getDescriptorLayout() const { return mDescriptorSetLayout; }

    [[nodiscard]] auto getDescriptorSet(int frameCount) const { return mDescriptorSet->getDescriptorSet(frameCount); }

private:
    Wrapper::Device::Ptr mDevice{ nullptr };

    std::vector<Wrapper::UniformParameter::Ptr> mUniformParams;

    Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout{ nullptr };
    Wrapper::DescriptorPool::Ptr      mDescriptorPool{ nullptr };
    Wrapper::DescriptorSet::Ptr       mDescriptorSet{ nullptr };
};
