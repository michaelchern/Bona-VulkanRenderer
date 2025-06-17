#pragma once

#include "base.h"
#include "instance.h"
#include "windowSurface.h"

namespace LearnVulkan::Wrapper
{
    // 必需的设备扩展列表 (交换链 + 维护扩展)
    const std::vector<const char*> deviceRequiredExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,    // 支持交换链功能（帧缓冲管理）
        VK_KHR_MAINTENANCE1_EXTENSION_NAME  // 提供API改进和兼容性支持
    };

    // Vulkan 设备管理类（封装物理设备和逻辑设备）
    class Device
    {
    public:
        using Ptr = std::shared_ptr<Device>;

        static Ptr create(Instance::Ptr instance, WindowSurface::Ptr surface)
        {
            return std::make_shared<Device>(instance, surface);
        }

        // 构造函数：初始化设备选择过程
        Device(Instance::Ptr instance, WindowSurface::Ptr surface);

        // 析构函数：销毁逻辑设备
        ~Device();

        // 选择可用的物理设备（GPU）
        void pickPhysicalDevice();

        // 给物理设备评分（用于选择最佳GPU）
        int rateDevice(VkPhysicalDevice device);

        // 检查设备是否满足基本要求
        bool isDeviceSuitable(VkPhysicalDevice device);

        // 初始化设备的队列族（图形队列 + 呈现队列）
        void initQueueFamilies(VkPhysicalDevice device);

        // 创建逻辑设备（VkDevice）
        void createLogicalDevice();

        // 检查队列族是否完整（图形+呈现）
        bool isQueueFamilyComplete();

        // === 访问方法 ===
        [[nodiscard]] auto getDevice() const { return mDevice; }                          // 获取逻辑设备
        [[nodiscard]] auto getPhysicalDevice() const { return mPhysicalDevice; }          // 获取物理设备

        [[nodiscard]] auto getGraphicQueueFamily() const { return mGraphicQueueFamily; }  // 图形队列族索引
        [[nodiscard]] auto getPresentQueueFamily() const { return mPresentQueueFamily; }  // 呈现队列族索引

        [[nodiscard]] auto getGraphicQueue() const { return mGraphicQueue; }              // 图形队列句柄
        [[nodiscard]] auto getPresentQueue() const { return mPresentQueue; }              // 呈现队列句柄
    private:
        VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };  // 物理设备句柄 (GPU)
        Instance::Ptr mInstance{ nullptr };                  // Vulkan 实例引用
        WindowSurface::Ptr mSurface{ nullptr };              // 窗口表面引用

        // 队列族管理
        std::optional<uint32_t> mGraphicQueueFamily;         // 图形队列族索引（可能未设置）
        VkQueue	mGraphicQueue{ VK_NULL_HANDLE };             // 图形队列句柄

        std::optional<uint32_t> mPresentQueueFamily;         // 呈现队列族索引（可能未设置）
        VkQueue mPresentQueue{ VK_NULL_HANDLE };             // 呈现队列句柄

        //逻辑设备
        VkDevice mDevice{ VK_NULL_HANDLE };                  // 逻辑设备句柄
    };
}