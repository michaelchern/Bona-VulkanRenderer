#pragma once

#include "base.h"

namespace LearnVulkan::Wrapper
{
    // 表示一个Vulkan实例的封装类
    // 通常用来表示渲染程序的元数据，比如 Vulkan 版本，是否启动 Debug，是否启用那些扩展等
    class Instance
    {
    public:
        // 智能指针别名，简化共享指针的使用
        using Ptr = std::shared_ptr<Instance>;

        // 静态工厂方法：创建Instance对象的共享指针
        // @param enableValidationLayer 是否启用Vulkan验证层（用于调试）
        static Ptr create(bool enableValidationLayer) { return std::make_shared<Instance>(enableValidationLayer); }

        // 构造函数：创建Vulkan实例
        // @param enableValidationLayer 控制验证层开关
        Instance(bool enableValidationLayer);

        // 析构函数：销毁Vulkan实例和调试工具
        ~Instance();

        // 打印所有可用的Vulkan扩展列表（调试用）
        void printAvailableExtensions();

        // 获取必需的Vulkan扩展列表
        // @return 返回const char*数组，包含GLFW等必要扩展名
        std::vector<const char*> getRequiredExtensions();

        // 检查请求的验证层是否可用
        // @return 验证层支持状态
        bool checkValidationLayerSupport();

        // 设置Vulkan调试回调（仅在启用验证层时有效）
        void setupDebugger();

        // 获取底层VkInstance对象
        // [[nodiscard]] 避免调用者忽略返回值
        [[nodiscard]] VkInstance getInstance() const { return mInstance; }

        // 获取验证层启用状态
        [[nodiscard]] bool getEnableValidationLayer() const { return mEnableValidationLayer; }

    private:
        VkInstance mInstance{ VK_NULL_HANDLE };                // Vulkan实例句柄
        bool mEnableValidationLayer{ false };                  // 验证层启用标志
        VkDebugUtilsMessengerEXT mDebugger{ VK_NULL_HANDLE };  // Vulkan调试信使句柄
    };
}