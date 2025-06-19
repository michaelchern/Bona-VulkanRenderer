
#include "commandPool.h"

namespace LearnVulkan::Wrapper
{
    /**
     * @brief 构造函数 - 创建Vulkan命令池
     *
     * @param device 关联的逻辑设备（封装对象）
     * @param flag 命令池创建标志（默认允许重置单个命令缓冲区）
     */
    CommandPool::CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag)
    {
        mDevice = device;  // 存储关联设备对象的智能指针

        // 配置命令池创建信息结构体
        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;  // 标准结构体类型

        // 从设备对象获取图形队列族索引（必须有效）
        createInfo.queueFamilyIndex = device->getGraphicQueueFamily().value();

        //指令修改的属性、指令池的内存属性
        //VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: 分配出来的CommandBuffer可以单独更新、单独重置
        //VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: 每一个CommandBuffer不能单独reset，必须集体重置，vkResetCommandPool
        createInfo.flags = flag;

        // 调用Vulkan API创建命令池
        if (vkCreateCommandPool(mDevice->getDevice(), &createInfo, nullptr, &mCommandPool) != VK_SUCCESS)
        {
            // 如果创建失败，抛出异常
            throw std::runtime_error("Error:  failed to create command pool");
        }
    }

    /**
     * @brief 析构函数 - 自动销毁Vulkan命令池
     *
     * 遵循RAII原则，当对象超出作用域时自动清理资源。
     * 注意：关联的设备对象必须比命令池存活更久
     */
    CommandPool::~CommandPool()
    {
        // 检查命令池句柄有效性
        if (mCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(mDevice->getDevice(), mCommandPool, nullptr);
        }
    }
}