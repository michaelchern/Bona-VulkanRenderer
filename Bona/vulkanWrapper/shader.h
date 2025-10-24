#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    class Shader
    {
    public:
        using Ptr = std::shared_ptr<Shader>;

        static Ptr create(const Device::Ptr& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint)
        {
            return std::make_shared<Shader>(device, fileName, shaderStage, entryPoint);
        }

        Shader(const Device::Ptr& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint);

        ~Shader();

        [[nodiscard]] auto  getShaderStage()      const { return mShaderStage; }
        [[nodiscard]] auto& getShaderEntryPoint() const { return mEntryPoint; }
        [[nodiscard]] auto  getShaderModule()     const { return mShaderModule; }

    private:
        VkShaderModule        mShaderModule{ VK_NULL_HANDLE };
        Device::Ptr           mDevice{ nullptr };
        std::string           mEntryPoint;
        VkShaderStageFlagBits mShaderStage;
    };
}