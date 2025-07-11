#pragma once

#include "buffer.h"
#include "../texture/texture.h"

namespace LearnVulkan::Wrapper
{
    struct UniformParameter
    {
        using Ptr = std::shared_ptr<UniformParameter>;
        static Ptr create() { return std::make_shared< UniformParameter>(); }

        size_t                   mSize{ 0 };
        uint32_t                 mBinding{ 0 };
        uint32_t                 mCount{ 0 };
        VkDescriptorType         mDescriptorType;
        VkShaderStageFlagBits    mStage;

        std::vector<Buffer::Ptr> mBuffers{};
        Texture::Ptr             mTexture{ nullptr };
    };
}