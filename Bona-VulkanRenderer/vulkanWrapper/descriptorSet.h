#pragma once

#include "base.h"
#include "device.h"
#include "description.h"
#include "descriptorSetLayout.h"
#include "descriptorPool.h"

namespace LearnVulkan::Wrapper
{
    /*
     * 对于每一个模型的渲染，都需要绑定一个 DescriptorSet，绑定的位置就是在 CommandBuffer
     * 一个 DescriptorSet 里面，都对应着一个 vp 矩阵使用的 buffer，一个 model 矩阵使用的 buffer，等等，其中也包括
     * binding size等等的描述信息
     * 由于交换链的存在，多帧有可能并行渲染，所以我们需要为每一个交换链的图片，对应生成一个DescriptorSet
     */

    class DescriptorSet
    {
    public:
        using Ptr = std::shared_ptr<DescriptorSet>;
        static Ptr create(const Device::Ptr& device,
                          const std::vector<UniformParameter::Ptr> params,
                          const DescriptorSetLayout::Ptr& layout,
                          const DescriptorPool::Ptr& pool,
                          int frameCount)
        {
            return std::make_shared<DescriptorSet>(device,
                                                   params,
                                                   layout,
                                                   pool,
                                                   frameCount);
        }

        DescriptorSet(const Device::Ptr &device,
                      const std::vector<UniformParameter::Ptr> params,
                      const DescriptorSetLayout::Ptr &layout,
                      const DescriptorPool::Ptr &pool,
                      int frameCount);

        ~DescriptorSet();

        [[nodiscard]] auto getDescriptorSet(int frameCount) const { return mDescriptorSets[frameCount]; }

    private:
        std::vector<VkDescriptorSet> mDescriptorSets{};
        Device::Ptr                  mDevice{ nullptr };
    };
}