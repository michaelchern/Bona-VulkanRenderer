#pragma once

#include "buffer.h"
//#include "../texture/texture.h"

namespace LearnVulkan::Wrapper
{

    struct UniformParameter
    {
        using Ptr = std::shared_ptr<UniformParameter>;
        static Ptr create() { return std::make_shared< UniformParameter>(); }

        size_t                  mSize{ 0 };
        uint32_t                mBinding{ 0 };

        //对于每一个binding点，都有可能传入不止一个uniform，可能是一个Matrix[]， count就代表了数组的大小, 
        //需要使用indexedDescriptor类型
        uint32_t                mCount{ 0 };
        VkDescriptorType        mDescriptorType;
        VkShaderStageFlagBits   mStage;

        std::vector<Buffer::Ptr> mBuffers{};
        Texture::Ptr mTexture{ nullptr };
    };
}