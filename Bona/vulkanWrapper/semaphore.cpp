#include "semaphore.h"

namespace LearnVulkan::Wrapper
{
    Semaphore::Semaphore(const Device::Ptr& device)
    {
        mDevice = device;

        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(mDevice->getDevice(), &createInfo, nullptr, &mSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create Semaphore!");
        }
    }

    Semaphore::~Semaphore()
    {
        if (mSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(mDevice->getDevice(), mSemaphore, nullptr);
        }
    }
}
