#pragma once

#include "base.h"
#include "device.h"
#include "description.h"

namespace LearnVulkan::Wrapper
{
    // 描述符池管理类：用于分配描述符集（Descriptor Sets）的Vulkan资源池
    class DescriptorPool
    {
    public:
        using Ptr = std::shared_ptr<DescriptorPool>;
        static Ptr create(const Device::Ptr& device) { return std::make_shared<DescriptorPool>(device); }

        // 构造函数（仅存储设备指针）
        DescriptorPool(const Device::Ptr &device);

        // 析构函数（自动销毁Vulkan描述符池）
        ~DescriptorPool();

        // 构建描述符池对象
        // params     - 绑定资源参数列表
        // frameCount - 交换链帧缓冲数量（决定池大小）
        void build(std::vector<UniformParameter::Ptr>& params, const int &frameCount);

        // 获取底层Vulkan描述符池对象
        [[nodiscard]] auto getPool() const { return mPool; }

    private:
        // 需要知道，每一种uniform都有多少个，才能为其预留分配空间，所谓空间并不是 uniformbuffer 的大小
        // 而是，根据每种uniform不同，那么描述符就不同，所以说空间指的是描述符的大小，蕴含在系统内部的

        VkDescriptorPool mPool{ VK_NULL_HANDLE };  // Vulkan描述符池句柄
        Device::Ptr      mDevice{ nullptr };       // Vulkan逻辑设备引用
    };
}