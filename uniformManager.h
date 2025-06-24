#pragma once

#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/description.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/commandPool.h"
#include "vulkanWrapper/base.h"

using namespace LearnVulkan;

// Uniform资源管理器：封装Uniform缓冲区和描述符的创建、更新与管理
class UniformManager
{
public:
    using Ptr = std::shared_ptr<UniformManager>;
    static Ptr create() { return std::make_shared<UniformManager>(); }

    UniformManager();   // 默认构造函数

    ~UniformManager();  // 析构函数

    // 初始化Uniform系统
    // 参数:
    //   device      - Vulkan设备
    //   commandPool - 命令池（用于缓冲区传输）
    //   frameCount  - 帧缓冲数量（双缓冲/三缓冲）
    void init(const Wrapper::Device::Ptr &device, const Wrapper::CommandPool::Ptr &commandPool, int frameCount);

    // 更新Uniform数据
    // 参数:
    //   vpMatrices    - 视图投影矩阵数据
    //   objectUniform - 对象模型矩阵数据
    //   frameCount    - 当前帧索引
    void update(const VPMatrices &vpMatrices, const ObjectUniform &objectUniform, const int& frameCount);

    // 获取描述符布局（用于创建图形管线）
    [[nodiscard]] auto getDescriptorLayout() const { return mDescriptorSetLayout; }

    // 获取当前帧的描述符集（用于命令缓冲区绑定）
    [[nodiscard]] auto getDescriptorSet(int frameCount) const { return mDescriptorSet->getDescriptorSet(frameCount); }

private:
    Wrapper::Device::Ptr mDevice{ nullptr };  // Vulkan设备

    // Uniform参数列表：定义绑定点配置
    std::vector<Wrapper::UniformParameter::Ptr> mUniformParams;

    Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout{ nullptr };  // 布局
    Wrapper::DescriptorPool::Ptr      mDescriptorPool{ nullptr };       // 池
    Wrapper::DescriptorSet::Ptr       mDescriptorSet{ nullptr };        // 集
};