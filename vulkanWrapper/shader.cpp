
#include "shader.h"

namespace LearnVulkan::Wrapper
{

    static std::vector<char> readBinary(const std::string& fileName)
    {
        std::ifstream file(fileName.c_str(), std::ios::ate | std::ios::binary | std::ios::in);

        if (!file)
        {
            throw std::runtime_error("Error: failed to open shader file");
        }

        const size_t fileSize = file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    Shader::Shader(const Device::Ptr& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint)
    {
        mDevice = device;
        mShaderStage = shaderStage;
        mEntryPoint = entryPoint;

        std::vector<char> codeBuffer = readBinary(fileName);

        VkShaderModuleCreateInfo shaderCreateInfo{};
        shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderCreateInfo.codeSize = codeBuffer.size();
        shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(codeBuffer.data());

        if (vkCreateShaderModule(mDevice->getDevice(), &shaderCreateInfo, nullptr, &mShaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create shader");
        }
    }

    Shader::~Shader()
    {
        if (mShaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(mDevice->getDevice(), mShaderModule, nullptr);
        }
    }
}