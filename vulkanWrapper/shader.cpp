
#include "shader.h"

namespace LearnVulkan::Wrapper
{
    /**
    * @brief 从文件中读取二进制数据（私有辅助函数）
    * @param fileName 要读取的文件路径
    * @return 包含文件二进制内容的字符向量
    *
    * 该函数使用ifstream以二进制模式读取文件：
    *  1. std::ios::ate - 初始定位到文件末尾
    *  2. std::ios::binary - 二进制模式（避免文本转换）
    *  3. std::ios::in - 输入/读取模式
    */
    static std::vector<char> readBinary(const std::string& fileName)
    {
        // 创建并打开文件流
        std::ifstream file(fileName.c_str(), std::ios::ate | std::ios::binary | std::ios::in);

        // 检查文件是否成功打开
        if (!file)
        {
            throw std::runtime_error("Error: failed to open shader file");
        }

        // 获取文件大小（因初始定位在末尾）
        const size_t fileSize = file.tellg();
        // 创建与文件大小匹配的缓冲区
        std::vector<char> buffer(fileSize);

        // 重置到文件开头读取内容
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    /**
    * @brief Shader类构造函数
    *
    * 执行流程:
    *  1. 初始化成员变量
    *  2. 读取SPIR-V二进制文件
    *  3. 创建Vulkan着色器模块
    *
    * @param device 关联的Vulkan逻辑设备（智能指针）
    * @param fileName SPIR-V二进制文件路径
    * @param shaderStage 着色器阶段（顶点/片段等）
    * @param entryPoint 着色器入口函数名
    */
    Shader::Shader(const Device::Ptr& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint)
    {
        // 初始化成员变量
        mDevice = device;            // 存储关联的Vulkan设备
        mShaderStage = shaderStage;  // 设置着色器阶段标识符
        mEntryPoint = entryPoint;    // 设置入口函数名称

        // 读取SPIR-V文件到内存中
        std::vector<char> codeBuffer = readBinary(fileName);

        // 配置着色器模块创建信息
        VkShaderModuleCreateInfo shaderCreateInfo{};
        shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;           // 标准结构类型
        shaderCreateInfo.codeSize = codeBuffer.size();                                  // SPIR-V代码字节数

        // 将char*转换为uint32_t*（SPIR-V要求32位字对齐）
        shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(codeBuffer.data());

        // 调用Vulkan API创建着色器模块
        // 注意：mDevice->getDevice() 获取底层VkDevice句柄
        if (vkCreateShaderModule(mDevice->getDevice(), &shaderCreateInfo, nullptr, &mShaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create shader");
        }
    }

    /**
    * @brief Shader类析构函数
    *
    * 按照RAII原则销毁分配的Vulkan资源：
    *  1. 检查着色器模块句柄是否有效
    *  2. 调用vkDestroyShaderModule销毁资源
    *  3. 避免在未初始化的句柄上调用销毁函数
    */
    Shader::~Shader()
    {
        // 确保着色器模块已被创建
        if (mShaderModule != VK_NULL_HANDLE)
        {
            // 使用关联设备销毁着色器模块
            vkDestroyShaderModule(mDevice->getDevice(), mShaderModule, nullptr);
        }
    }
}