#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    /**
     * @brief Vulkan命令池的封装类
     *
     * 命令池用于分配和管理命令缓冲区(VkCommandBuffer)，控制其内存分配策略。
     * 命令池与特定队列族(queue family)关联（需在Device创建时指定）。
     */
    class CommandPool
    {
    public:
        using Ptr = std::shared_ptr<CommandPool>;

        /**
         * @brief 创建命令池的工厂方法
         *
         * @param  device 关联的逻辑设备封装对象（Device）
         * @param  flag   创建标志（默认可重置命令缓冲区）
         * @return Ptr    返回命令池的共享智能指针
         */
        static Ptr create(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
        {
            return std::make_shared<CommandPool>(device, flag); 
        }

        /**
         * @brief 构造函数（需在.cpp实现中创建底层VkCommandPool）
         *
         * @param device 关联的逻辑设备
         * @param flag   控制命令池行为的标志（见Vulkan规范）
         */
        CommandPool(const Device::Ptr &device, VkCommandPoolCreateFlagBits flag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        /// @brief 析构函数（自动销毁VkCommandPool）
        ~CommandPool();

        /// @brief 获取底层VkCommandPool句柄
        [[nodiscard]] auto getCommandPool() const { return mCommandPool; }

    private:
        VkCommandPool mCommandPool{ VK_NULL_HANDLE };  // 底层Vulkan命令池对象
        Device::Ptr   mDevice{ nullptr };              // 关联的逻辑设备（智能指针管理生命周期）
    };
}