
#include "semaphore.h"

namespace LearnVulkan::Wrapper
{
    /**
     * @brief 构造函数 - 创建Vulkan信号量
     *
     * @param device 关联的逻辑设备对象（封装类）
     */
    Semaphore::Semaphore(const Device::Ptr& device)
    {
        mDevice = device;  // 存储关联的逻辑设备对象

        // 配置信号量创建信息
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;  // 标准Vulkan结构体类型

        // 调用Vulkan API创建信号量
        if (vkCreateSemaphore(mDevice->getDevice(), &createInfo, nullptr, &mSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create Semaphore!");
        }
    }

    /**
     * @brief 析构函数 - 销毁Vulkan信号量
     *
     * 遵循RAII原则，当对象超出作用域时自动清理资源
     * 注意：关联的设备对象必须比信号量存活更久
     */
    Semaphore::~Semaphore()
    {
        if (mSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(mDevice->getDevice(), mSemaphore, nullptr);
        }
    }
}