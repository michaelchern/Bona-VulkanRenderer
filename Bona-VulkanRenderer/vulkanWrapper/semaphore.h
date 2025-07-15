#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    class Semaphore
    {
    public:
        using Ptr = std::shared_ptr<Semaphore>;
        static Ptr create(const Device::Ptr &device) { return std::make_shared<Semaphore>(device); }

        Semaphore(const Device::Ptr& device);

        ~Semaphore();

        [[nodiscard]] auto getSemaphore() const { return mSemaphore; }

    private:
        VkSemaphore mSemaphore{ VK_NULL_HANDLE };  
        Device::Ptr mDevice{ nullptr };            
    };
}