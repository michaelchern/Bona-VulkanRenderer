#pragma once

#include "base.h"
#include "device.h"
#include "description.h"

namespace LearnVulkan::Wrapper
{
    // 描述符集布局类，用于管理Vulkan中的描述符集布局资源
    class DescriptorSetLayout
    {
    public:
        using Ptr = std::shared_ptr<DescriptorSetLayout>;
        static Ptr create(const Device::Ptr& device) { return std::make_shared<DescriptorSetLayout>(device); }

        // 构造函数
        // 参数 device - Vulkan逻辑设备指针
        DescriptorSetLayout(const Device::Ptr &device);

        // 析构函数：自动清理Vulkan资源
        ~DescriptorSetLayout();

        // 构建描述符集布局对象
        // 参数 params - 包含绑定点信息的Uniform参数列表
        void build(const std::vector<UniformParameter::Ptr>& params);

        // 获取底层Vulkan布局对象
        // 返回 VkDescriptorSetLayout句柄
        [[nodiscard]] auto getLayout() const { return mLayout; }

    private:
        VkDescriptorSetLayout mLayout{ VK_NULL_HANDLE };  // Vulkan描述符集布局句柄
        Device::Ptr mDevice{ nullptr };                   // Vulkan逻辑设备引用

        // 存储关联的Uniform参数列表
        std::vector<UniformParameter::Ptr> mParams{};
    };
}