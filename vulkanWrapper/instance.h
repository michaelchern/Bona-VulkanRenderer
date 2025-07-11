#pragma once

#include "base.h"

namespace LearnVulkan::Wrapper
{
    // 表示一个Vulkan实例的封装类
    // 通常用来表示渲染程序的元数据，比如 Vulkan 版本，是否启动 Debug，是否启用那些扩展等
    class Instance
    {
    public:
        using Ptr = std::shared_ptr<Instance>;

        static Ptr create(bool enableValidationLayer) { return std::make_shared<Instance>(enableValidationLayer); }

        Instance(bool enableValidationLayer);

        ~Instance();

        void printAvailableExtensions();

        std::vector<const char*> getRequiredExtensions();

        bool checkValidationLayerSupport();

        void setupDebugger();

        [[nodiscard]] VkInstance getInstance() const { return mInstance; }

        [[nodiscard]] bool getEnableValidationLayer() const { return mEnableValidationLayer; }

    private:
        VkInstance               mInstance{ VK_NULL_HANDLE };
        bool                     mEnableValidationLayer{ false };
        VkDebugUtilsMessengerEXT mDebugger{ VK_NULL_HANDLE };
    };
}