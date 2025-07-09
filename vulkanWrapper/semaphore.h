#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    /**
     * @class Semaphore
     * @brief Vulkan信号量的RAII封装类
     *
     * 用于管理GPU操作间的同步，特别是在以下场景：
     * 1. 交换链图像获取（vkAcquireNextImageKHR）完成通知
     * 2. 渲染命令提交完成通知
     * 3. 队列操作间的执行依赖管理
     */
    class Semaphore
    {
    public:
        using Ptr = std::shared_ptr<Semaphore>;
        static Ptr create(const Device::Ptr &device) { return std::make_shared<Semaphore>(device); }

        /**
         * @brief 构造函数 - 创建Vulkan信号量
         * @param device 关联的Vulkan逻辑设备
         */
        Semaphore(const Device::Ptr& device);

        /**
         * @brief 析构函数 - 自动销毁信号量
         */
        ~Semaphore();

        // 获取底层Vulkan信号量句柄（用于API调用）
        [[nodiscard]] auto getSemaphore() const { return mSemaphore; }
    private:
        VkSemaphore mSemaphore{ VK_NULL_HANDLE };  // Vulkan信号量句柄
        Device::Ptr mDevice{ nullptr };            // 关联的逻辑设备（弱引用）
    };
}