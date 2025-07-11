
#include "uniformManager.h"

UniformManager::UniformManager()
{

}


UniformManager::~UniformManager()
{

}

void UniformManager::init(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, int frameCount)
{
    mDevice = device;

    auto vpParam             = Wrapper::UniformParameter::create();  
    vpParam->mBinding        = 0;                                    
    vpParam->mCount          = 1;                                   
    vpParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vpParam->mSize           = sizeof(VPMatrices);
    vpParam->mStage          = VK_SHADER_STAGE_VERTEX_BIT;

    for (int i = 0; i < frameCount; ++i)
    {
        auto buffer = Wrapper::Buffer::createUniformBuffer(device, vpParam->mSize, nullptr);
        vpParam->mBuffers.push_back(buffer);
    }
    mUniformParams.push_back(vpParam);

    auto objectParam             = Wrapper::UniformParameter::create();
    objectParam->mBinding        = 1;
    objectParam->mCount          = 1;
    objectParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    objectParam->mSize           = sizeof(ObjectUniform);
    objectParam->mStage          = VK_SHADER_STAGE_VERTEX_BIT;

    for (int i = 0; i < frameCount; ++i)
    {
        auto buffer = Wrapper::Buffer::createUniformBuffer(device, objectParam->mSize, nullptr);
        objectParam->mBuffers.push_back(buffer);
    }
    mUniformParams.push_back(objectParam);

    auto textureParam             = Wrapper::UniformParameter::create();
    textureParam->mBinding        = 2;
    textureParam->mCount          = 1;
    textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureParam->mStage          = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureParam->mTexture        = Texture::create(mDevice, commandPool, "assets/dragonBall.jpg");
    mUniformParams.push_back(textureParam);

    mDescriptorSetLayout = Wrapper::DescriptorSetLayout::create(device);
    mDescriptorSetLayout->build(mUniformParams);

    mDescriptorPool = Wrapper::DescriptorPool::create(device);
    mDescriptorPool->build(mUniformParams, frameCount);

    mDescriptorSet = Wrapper::DescriptorSet::create(device, mUniformParams, mDescriptorSetLayout, mDescriptorPool, frameCount);
}

void UniformManager::update(const VPMatrices& vpMatrices, const ObjectUniform& objectUniform, const int& frameCount)
{
    mUniformParams[0]->mBuffers[frameCount]->updateBufferByMap(
        (void*)(&vpMatrices),  // 数据源指针
        sizeof(VPMatrices)     // 数据大小
    );

    // 2. 更新当前帧的模型矩阵缓冲区
    //   mUniformParams[1] - 模型矩阵参数
    mUniformParams[1]->mBuffers[frameCount]->updateBufferByMap(
        (void*)(&objectUniform),  // 数据源指针
        sizeof(ObjectUniform)     // 数据大小
    );

    // 注意：纹理不需要每帧更新，初始设置后即保持
}
