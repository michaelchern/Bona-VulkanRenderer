#pragma once

#include "buffer.h"
//#include "../texture/texture.h"

namespace LearnVulkan::Wrapper
{
    // Uniform参数结构体：描述描述符绑定的配置信息
    struct UniformParameter
    {
        using Ptr = std::shared_ptr<UniformParameter>;
        static Ptr create() { return std::make_shared< UniformParameter>(); }

        size_t                  mSize{ 0 };            // 数据段大小（字节数）
        uint32_t                mBinding{ 0 };         // 着色器中的绑定点索引（对应layout(binding=X)）

        //对于每一个binding点，都有可能传入不止一个uniform，可能是一个Matrix[]， count就代表了数组的大小, 
        //需要使用indexedDescriptor类型

        /*
        * 数组类型支持：当描述符包含数组时（如uniform数组或纹理数组）
        * - mCount > 1：表示是数组类型描述符
        * - 需要对应使用 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC 等索引描述符类型
        */
        uint32_t                mCount{ 0 };

        VkDescriptorType        mDescriptorType;       // 描述符类型（如UBO/SSBO/Sampler等）
        VkShaderStageFlagBits   mStage;                // 可见的着色器阶段（VERTEX/FRAGMENT等）

        // 关联资源
        std::vector<Buffer::Ptr> mBuffers{};           // 绑定的缓冲区列表（用于UBO/SSBO等）
        Texture::Ptr             mTexture{ nullptr };  // 绑定的纹理资源（用于采样器/存储图像等）
    };
}