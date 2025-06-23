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
        //需要知道，每一种uniform都有多少个，才能为其预留分配空间，所谓空间并不是uniformbuffer的大小
        //而是，根据每种uniform不同，那么描述符就不同，所以说空间指的是描述符的大小，蕴含在系统内部的
        VkDescriptorPool mPool{ VK_NULL_HANDLE };
        Device::Ptr mDevice{ nullptr };
    };
}