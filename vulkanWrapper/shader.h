#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{

    /**
    * @class Shader
    * @brief 封装和管理Vulkan着色器模块资源
    *
    * 该类负责：
    *  - 从SPIR-V文件加载着色器代码
    *  - 创建VkShaderModule
    *  - 管理着色器入口点和阶段信息
    *  - 自动销毁分配的Vulkan资源
    */
    class Shader
    {
    public:
        using Ptr = std::shared_ptr<Shader>;

        /**
        * @brief 创建Shader对象的工厂函数
        * @param device 关联的Vulkan逻辑设备
        * @param fileName SPIR-V二进制文件路径
        * @param shaderStage 着色器阶段标识（VK_SHADER_STAGE_*）
        * @param entryPoint 着色器入口函数名（默认为"main"）
        * @return 指向新Shader对象的共享指针
        */
        static Ptr create(const Device::Ptr& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint)
        {
            return std::make_shared<Shader>(device, fileName, shaderStage, entryPoint);
        }

        Shader(const Device::Ptr& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint);

        ~Shader();

        [[nodiscard]] auto getShaderStage() const { return mShaderStage; }
        [[nodiscard]] auto& getShaderEntryPoint() const { return mEntryPoint; }
        [[nodiscard]] auto getShaderModule() const { return mShaderModule; }

    private:
        VkShaderModule mShaderModule{ VK_NULL_HANDLE };
        Device::Ptr mDevice{ nullptr };
        std::string mEntryPoint;
        VkShaderStageFlagBits mShaderStage;
    };
}