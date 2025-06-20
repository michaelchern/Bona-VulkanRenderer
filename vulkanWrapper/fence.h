#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    /*
    * fence是控制一次队列提交的标志，与semaphore区别，semaphore控制单一命令提交信息内的
    * 不同执行阶段之间的依赖关系，semaphore无法手动用API去激发的
    * fence控制一个队列（GraphicQueue）里面一次性提交的所有指令执行完毕
    * 分为激发态/非激发态,并且可以进行API级别的控制
    *
    *
    * @class Fence
    * @brief Vulkan栅栏的RAII封装类
    *
    * 用于CPU-GPU间的同步机制，主要功能：
    * - 监控GPU任务执行状态
    * - 提供CPU等待GPU任务完成的机制
    * - 支持重置状态以重复使用
    *
    * 与信号量(Semaphore)的区别：
    * - 栅栏(Fence): CPU可查询/控制状态 (激发态=任务完成)
    * - 信号量(Semaphore): GPU操作间同步 (CPU不可直接访问)
    */
    class Fence
    {
    public:
        using Ptr = std::shared_ptr<Fence>;
        static Ptr create(const Device::Ptr& device, bool signaled = true)
        {
            return std::make_shared<Fence>(device, signaled);
        }

        /**
        * @brief 构造函数 - 创建Vulkan栅栏
        *
        * @param device 关联的逻辑设备
        * @param signaled 初始状态 (true=创建即为激发态)
        */
        Fence(const Device::Ptr& device, bool signaled = true);

        /// 析构函数 - 自动销毁栅栏
        ~Fence();

        /// 重置栅栏为非激发态 (准备重用)
        void resetFence();

        /**
        * @brief 阻塞等待栅栏变为激发态
        *
        * @param timeout 超时时间(ns)，默认永久等待
        * @throws std::runtime_error 等待超时时抛出异常
        */
        void block(uint64_t timeout = UINT64_MAX);

        /// 获取底层VkFence句柄
        [[nodiscard]] auto getFence() const { return mFence; }
    private:
        VkFence mFence{ VK_NULL_HANDLE };  // Vulkan栅栏句柄
        Device::Ptr mDevice{ nullptr };    // 关联的逻辑设备
    };
}