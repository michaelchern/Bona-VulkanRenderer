#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    class Fence
    {
    public:
        using Ptr = std::shared_ptr<Fence>;
        static Ptr create(const Device::Ptr& device, bool signaled = true)
        {
            return std::make_shared<Fence>(device, signaled);
        }

        Fence(const Device::Ptr& device, bool signaled = true);

        ~Fence();

        void resetFence();

        void block(uint64_t timeout = UINT64_MAX);

        [[nodiscard]] auto getFence() const { return mFence; }

    private:
        VkFence mFence{ VK_NULL_HANDLE };  
        Device::Ptr mDevice{ nullptr };    
    };
}