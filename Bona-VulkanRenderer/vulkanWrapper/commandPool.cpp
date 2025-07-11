#include "commandPool.h"

namespace LearnVulkan::Wrapper
{
    CommandPool::CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag)
    {
        mDevice = device;

        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.queueFamilyIndex = device->getGraphicQueueFamily().value();
        createInfo.flags            = flag;

        if (vkCreateCommandPool(mDevice->getDevice(), &createInfo, nullptr, &mCommandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create command pool!");
        }
    }

    CommandPool::~CommandPool()
    {
        if (mCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(mDevice->getDevice(), mCommandPool, nullptr);
        }
    }
}