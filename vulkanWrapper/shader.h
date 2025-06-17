#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{

    /**
    * @class Shader
    * @brief 封装和管理Vulkan着色器模块（VkShaderModule）的生命周期
    *
    * 核心职责：
    *  1. 从SPIR-V二进制文件加载着色器字节码
    *  2. 创建和管理VkShaderModule对象
    *  3. 存储着色器入口点和阶段信息
    *  4. 遵循RAII原则，自动销毁资源
    */
    class Shader
    {
    public:
        using Ptr = std::shared_ptr<Shader>;

        /**
        * @brief 创建Shader的工厂方法（安全封装）
        * @param device    Vulkan逻辑设备封装（智能指针）
        * @param fileName  SPIR-V二进制文件路径（如"shaders/vert.spv"）
        * @param shaderStage 着色器阶段标志（VK_SHADER_STAGE_VERTEX_BIT等）
        * @param entryPoint  着色器入口函数名（默认"main"）
        * @return 新Shader对象的共享指针
        */
        static Ptr create(const Device::Ptr& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint)
        {
            return std::make_shared<Shader>(device, fileName, shaderStage, entryPoint);
        }

        // 构造函数（实际执行加载和创建着色器模块）
        Shader(const Device::Ptr& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint);

        // 析构函数（自动销毁Vulkan资源）
        ~Shader();

        [[nodiscard]] auto getShaderStage() const { return mShaderStage; }       // 获取着色器阶段
        [[nodiscard]] auto& getShaderEntryPoint() const { return mEntryPoint; }  // 获取入口点名称
        [[nodiscard]] auto getShaderModule() const { return mShaderModule; }     // 获取原始VkShaderModule句柄

    private:
        VkShaderModule mShaderModule{ VK_NULL_HANDLE };  // Vulkan着色器模块句柄（初始化为空）
        Device::Ptr mDevice{ nullptr };                  // 关联的逻辑设备（智能指针）
        std::string mEntryPoint;                         // 着色器入口函数（如"main"）
        VkShaderStageFlagBits mShaderStage;              // 着色器阶段（顶点/片段等）
    };
}