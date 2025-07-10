
#include "uniformManager.h"

// 构造函数
UniformManager::UniformManager()
{
    // 空实现，初始化工作放在init()
}

// 析构函数：使用RAII管理，成员智能指针会自动清理
UniformManager::~UniformManager()
{
    // 空实现，依赖智能指针自动管理资源
}

// 初始化Uniform资源系统
void UniformManager::init(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, int frameCount)
{
    mDevice = device;

    // 1. 创建VP矩阵Uniform参数和缓冲区 -----------------------------
    auto vpParam             = Wrapper::UniformParameter::create();  
    vpParam->mBinding        = 0;                                    // 绑定点0（对应shader中的binding=0）
    vpParam->mCount          = 1;                                    // 单缓冲区（非数组）
    vpParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;    // UBO类型
    vpParam->mSize           = sizeof(VPMatrices);                   // 视图投影矩阵结构大小
    vpParam->mStage          = VK_SHADER_STAGE_VERTEX_BIT;           // 顶点着色器可见

    // 为每帧创建独立的VP矩阵缓冲区（多帧支持）
    for (int i = 0; i < frameCount; ++i)
    {
        // 创建设备本地Uniform缓冲区
        auto buffer = Wrapper::Buffer::createUniformBuffer(device, vpParam->mSize, nullptr);
        vpParam->mBuffers.push_back(buffer);
    }
    mUniformParams.push_back(vpParam);  // 添加到参数列表

    // 2. 创建模型矩阵Uniform参数和缓冲区 ----------------------------
    auto objectParam             = Wrapper::UniformParameter::create();
    objectParam->mBinding        = 1;                                    // 绑定点1（对应shader中的binding=1）
    objectParam->mCount          = 1;
    objectParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    objectParam->mSize           = sizeof(ObjectUniform);                // 模型矩阵结构大小
    objectParam->mStage          = VK_SHADER_STAGE_VERTEX_BIT;

    // 为每帧创建独立的模型矩阵缓冲区
    for (int i = 0; i < frameCount; ++i)
    {
        auto buffer = Wrapper::Buffer::createUniformBuffer(device, objectParam->mSize, nullptr);
        objectParam->mBuffers.push_back(buffer);
    }
    mUniformParams.push_back(objectParam);  // 添加到参数列表

    // 3. 创建纹理采样器Uniform参数 --------------------------------
    auto textureParam             = Wrapper::UniformParameter::create();
    textureParam->mBinding        = 2;                                          // 绑定点2（对应shader中的binding=2）
    textureParam->mCount          = 1;
    textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;  // 纹理采样器类型
    textureParam->mStage          = VK_SHADER_STAGE_FRAGMENT_BIT;               // 片段着色器可见
    textureParam->mTexture        = Texture::create(mDevice, commandPool, "assets/dragonBall.jpg");
    mUniformParams.push_back(textureParam);  // 添加到参数列表

    // 4. 创建描述符系统组件 -------------------------------------

    // 4.1 创建描述符布局（定义绑定点结构）
    mDescriptorSetLayout = Wrapper::DescriptorSetLayout::create(device);
    mDescriptorSetLayout->build(mUniformParams);

    // 4.2 创建描述符池（分配资源池）
    mDescriptorPool = Wrapper::DescriptorPool::create(device);
    mDescriptorPool->build(mUniformParams, frameCount);

    // 4.3 创建描述符集（实际绑定资源）
    mDescriptorSet = Wrapper::DescriptorSet::create(device, mUniformParams, mDescriptorSetLayout, mDescriptorPool, frameCount);
}

// 更新Uniform数据（每帧调用）
void UniformManager::update(const VPMatrices& vpMatrices, const ObjectUniform& objectUniform, const int& frameCount)
{
    // 1. 更新当前帧的VP矩阵缓冲区
    //   mUniformParams[0]    - VP矩阵参数
    //   mBuffers[frameCount] - 当前帧对应的缓冲区
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
