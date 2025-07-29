#pragma once

#include "base.h"
#include "instance.h"
#include "windowSurface.h"

namespace LearnVulkan::Wrapper
{
    const std::vector<const char*> deviceRequiredExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME
    };

    class Device
    {
    public:
        using Ptr = std::shared_ptr<Device>;

        static Ptr create(Instance::Ptr& instance, WindowSurface::Ptr& surface)
        {
            return std::make_shared<Device>(instance, surface);
        }

        Device(Instance::Ptr& instance, WindowSurface::Ptr& surface);

        ~Device();

        void pickPhysicalDevice();

        int rateDevice(VkPhysicalDevice device);

        bool isDeviceSuitable(VkPhysicalDevice device);

        void initQueueFamilies(VkPhysicalDevice device);

        void createLogicalDevice();

        bool isQueueFamilyComplete();

		VkSampleCountFlags getMaxUsableSampleCount();

        [[nodiscard]] auto getDevice()             const { return mDevice; }
        [[nodiscard]] auto getPhysicalDevice()     const { return mPhysicalDevice; }
        [[nodiscard]] auto getGraphicQueueFamily() const { return mGraphicQueueFamily; }
        [[nodiscard]] auto getPresentQueueFamily() const { return mPresentQueueFamily; }
        [[nodiscard]] auto getGraphicQueue()       const { return mGraphicQueue; }
        [[nodiscard]] auto getPresentQueue()       const { return mPresentQueue; }

    private:
        VkPhysicalDevice   mPhysicalDevice{ VK_NULL_HANDLE };
        Instance::Ptr      mInstance{ nullptr };
        WindowSurface::Ptr mSurface{ nullptr };

        std::optional<uint32_t> mGraphicQueueFamily;
        VkQueue                 mGraphicQueue{ VK_NULL_HANDLE };

        std::optional<uint32_t> mPresentQueueFamily;
        VkQueue                 mPresentQueue{ VK_NULL_HANDLE };

        VkDevice mDevice{ VK_NULL_HANDLE };

        VkSampleCountFlagBits mMsaaSamples{ VK_SAMPLE_COUNT_1_BIT };
    };
}