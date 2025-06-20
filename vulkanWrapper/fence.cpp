
#include "fence.h"

namespace LearnVulkan::Wrapper
{
    /**
    * @brief 构造函数 - 创建Vulkan栅栏
    *
    * @param device 关联的逻辑设备
    * @param signaled 栅栏初始状态 (true=创建即为激发态, false=非激发态)
    */
    Fence::Fence(const Device::Ptr& device, bool signaled)
    {
        mDevice = device;  // 存储关联设备对象

        // 配置栅栏创建信息
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;  // 标准结构体类型

        // 设置初始状态标志：如果signaled=true，则创建即为激发态
        createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        // 调用Vulkan API创建栅栏
        if (vkCreateFence(mDevice->getDevice(), &createInfo, nullptr, &mFence) != VK_SUCCESS)
        {
            throw std::runtime_error("Error:failed to create fence");
        }
    }

    /// 析构函数 - 自动销毁栅栏资源
    Fence::~Fence()
    {
        if (mFence != VK_NULL_HANDLE)
        {
            vkDestroyFence(mDevice->getDevice(), mFence, nullptr);
        }
    }

    /**
    * @brief 重置栅栏为非激发态
    *
    * 将栅栏从激发态(已触发)重置为非激发态(未触发)，
    * 通常在重复使用栅栏前调用。
    */
    void Fence::resetFence()
    {
        vkResetFences(mDevice->getDevice(), 1, &mFence);
    }

    /**
    * @brief 阻塞等待栅栏变为激发态
    *
    * @param timeout 超时时间(纳秒)，默认永久等待(UINT64_MAX)
    *
    * 注意：当前实现直接使用Vulkan API的阻塞等待，
    * 但未处理超时错误(应抛出异常)
    */
    void Fence::block(uint64_t timeout)
    {
        vkWaitForFences(
            mDevice->getDevice(),  
            1,                     // 等待的栅栏数量
            &mFence,               // 栅栏对象数组
            VK_TRUE,               // 等待所有栅栏触发
            timeout                // 超时时间(纳秒)
        );
    }
}