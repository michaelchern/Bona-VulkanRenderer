#pragma once

#include "base.h"
#include "device.h"
#include "description.h"
#include "descriptorSetLayout.h"
#include "descriptorPool.h"

namespace LearnVulkan::Wrapper
{
    /*
    * 对于每一个模型的渲染，都需要绑定一个DescriptorSet，绑定的位置就是在CommandBuffer
    * 一个DescriptorSet里面，都对应着一个vp矩阵使用的buffer，一个model矩阵使用的buffer，等等,其中也包括
    * binding size等等的描述信息
    * 由于交换链的存在，多帧有可能并行渲染，所以我们需要为每一个交换链的图片，对应生成一个DescriptorSet
    */

    /*
    * 描述符集管理类：负责创建和管理描述符集
    *
    * 核心功能：
    * - 为每个模型/渲染对象创建描述符集
    * - 支持多帧渲染（交换链图像并行处理）
    * - 封装Vulkan资源绑定逻辑
    *
    * 设计背景：
    * 1. 每个渲染对象需要独立的描述符集（例如绑定不同的MVP矩阵）
    * 2. 为避免帧间资源冲突，需要为每个交换链图像创建独立的描述符集
    * 3. 提供简单接口访问帧对应的描述符集
    */
    class DescriptorSet
    {
    public:
        using Ptr = std::shared_ptr<DescriptorSet>;
        static Ptr create(
            const Device::Ptr& device,
            const std::vector<UniformParameter::Ptr> params,  // 资源绑定参数列表
            const DescriptorSetLayout::Ptr& layout,           // 描述符集布局
            const DescriptorPool::Ptr& pool,                  // 描述符池
            int frameCount)                                   // 帧缓冲数量
        { 
            return std::make_shared<DescriptorSet>(
                device,
                params,
                layout,
                pool,
                frameCount
            ); 
        }

        // 构造函数（执行核心创建逻辑）
        DescriptorSet(
            const Device::Ptr &device,
            const std::vector<UniformParameter::Ptr> params,
            const DescriptorSetLayout::Ptr &layout,
            const DescriptorPool::Ptr &pool,
            int frameCount
        );

        // 析构函数
        ~DescriptorSet();

        // 获取指定帧对应的描述符集句柄
        [[nodiscard]] auto getDescriptorSet(int frameCount) const { return mDescriptorSets[frameCount]; }

    private:
        std::vector<VkDescriptorSet> mDescriptorSets{};   // 存储各帧描述符集的数组
        Device::Ptr                  mDevice{ nullptr };  // Vulkan设备引用
    };
}