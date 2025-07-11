#include "device.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{

    Device::Device(Instance::Ptr instance, WindowSurface::Ptr surface)
    {
        mInstance = instance;
        mSurface = surface;
        pickPhysicalDevice();
        initQueueFamilies(mPhysicalDevice);
        createLogicalDevice();
    }

    Device::~Device()
    {
        vkDestroyDevice(mDevice, nullptr);
        mSurface.reset();
        mInstance.reset();
    }

    // 选择最佳物理设备
    void Device::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;

        // 1. 获取可用物理设备数量
        vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("Error:failed to enumeratePhysicalDevice");
        }

        // 2. 获取所有物理设备句柄
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, devices.data());

        // 3. 使用multimap自动按评分排序设备
        std::multimap<int, VkPhysicalDevice> candidates;
        for (const auto& device : devices)
        {
            // 给设备评分
            int score = rateDevice(device);
            candidates.insert(std::make_pair(score, device));
        }

        // 4. 选择最高分且合适的设备
        if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second))
        {
            mPhysicalDevice = candidates.rbegin()->second;
        }

        if (mPhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Error:failed to get physical device");
        }
    }

    // 设备评分函数
    int Device::rateDevice(VkPhysicalDevice device)
    {
        int score = 0;

        // 1. 获取设备属性（名称、类型、版本等）
        VkPhysicalDeviceProperties  deviceProp;
        vkGetPhysicalDeviceProperties(device, &deviceProp);

        // 2. 获取设备特性（支持的功能）
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // 3. 独立显卡加分（性能更好）
        if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 1000;
        }

        // 4. 支持更大纹理加分
        score += deviceProp.limits.maxImageDimension2D;

        // 5. 必须有几何着色器支持
        if (!deviceFeatures.geometryShader)
        {
            return 0;
        }

        return score;
    }

    // 检查设备是否满足最低要求
    bool Device::isDeviceSuitable(VkPhysicalDevice device)
    {
        // 1. 获取设备属性
        VkPhysicalDeviceProperties  deviceProp;
        vkGetPhysicalDeviceProperties(device, &deviceProp);

        // 2. 获取设备特性
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // 3. 必须满足的条件：
        return deviceProp.deviceType ==
            VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&  // 独立显卡
            deviceFeatures.geometryShader &&         // 支持几何着色器
            deviceFeatures.samplerAnisotropy;        // 支持各向异性过滤
    }

    // 初始化队列族索引
    void Device::initQueueFamilies(VkPhysicalDevice device)
    {
        uint32_t queueFamilyCount = 0;

        // 1. 获取队列族数量
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // 2. 获取队列族属性
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            // 3. 查找支持图形操作的队列族
            if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                mGraphicQueueFamily = i;
            }

            // 4. 查找支持表面呈现的队列族
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface->getSurface(), &presentSupport);

            if (presentSupport)
            {
                mPresentQueueFamily = i;
            }

            // 5. 如果找到两种队列，提前结束
            if (isQueueFamilyComplete()) {
                break;
            }

            ++i;
        }
    }

    // 创建逻辑设备
    void Device::createLogicalDevice()
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        // 1. 收集需要创建的队列族（去重）
        std::set<uint32_t> queueFamilies = { mGraphicQueueFamily.value(), mPresentQueueFamily.value() };

        float queuePriority = 1.0;  // 队列优先级

        // 2. 为每个队列族创建队列信息
        for (uint32_t queueFamily : queueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        // 3. 启用设备特性（各向异性过滤）
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        // 4. 填写逻辑设备创建信息
        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        // 5. 启用设备扩展
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceRequiredExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceRequiredExtensions.data();

        // 6. 启用验证层（如果实例启用了）
        if (mInstance->getEnableValidationLayer())
        {
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        // 7. 创建逻辑设备
        if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS)
        {
            throw std::runtime_error("Error:failed to create logical device");
        }

        // 8. 获取队列句柄
        vkGetDeviceQueue(mDevice, mGraphicQueueFamily.value(), 0, &mGraphicQueue);
        vkGetDeviceQueue(mDevice, mPresentQueueFamily.value(), 0, &mPresentQueue);
    }

    bool Device::isQueueFamilyComplete()
    {
        return mGraphicQueueFamily.has_value() && mPresentQueueFamily.has_value();
    }

    VkSampleCountFlags Device::getMaxUsableSampleCount()
    {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(mPhysicalDevice, &props);

        VkSampleCountFlags counts = std::min(props.limits.framebufferColorSampleCounts,
                                             props.limits.framebufferDepthSampleCounts);

        if (counts & VK_SAMPLE_COUNT_64_BIT)
        {
            return VK_SAMPLE_COUNT_64_BIT;
        }

        if (counts & VK_SAMPLE_COUNT_32_BIT)
        {
            return VK_SAMPLE_COUNT_32_BIT;
        }

        if (counts & VK_SAMPLE_COUNT_16_BIT)
        {
            return VK_SAMPLE_COUNT_16_BIT;
        }

        if (counts & VK_SAMPLE_COUNT_8_BIT)
        {
            return VK_SAMPLE_COUNT_8_BIT;
        }

        if (counts & VK_SAMPLE_COUNT_4_BIT)
        {
            return VK_SAMPLE_COUNT_4_BIT;
        }

        if (counts & VK_SAMPLE_COUNT_2_BIT)
        {
            return VK_SAMPLE_COUNT_2_BIT;
        }

        return VK_SAMPLE_COUNT_1_BIT;
    }
}
