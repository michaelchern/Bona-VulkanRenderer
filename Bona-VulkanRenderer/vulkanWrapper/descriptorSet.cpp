
#include "descriptorSet.h"

namespace LearnVulkan::Wrapper
{
    // 描述符集构造函数：创建并配置多个描述符集
    DescriptorSet::DescriptorSet(const Device::Ptr& device,                        // Vulkan设备
                                 const std::vector<UniformParameter::Ptr> params,  // 绑定资源参数列表
                                 const DescriptorSetLayout::Ptr& layout,           // 描述符集布局
                                 const DescriptorPool::Ptr& pool,                  // 描述符池
                                 int frameCount)                                   // 帧缓冲数量
    {
        mDevice = device;

        std::vector<VkDescriptorSetLayout> layouts(frameCount, layout->getLayout());

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = pool->getPool();
        allocInfo.descriptorSetCount = frameCount;
        allocInfo.pSetLayouts        = layouts.data();

        mDescriptorSets.resize(frameCount);

        if (vkAllocateDescriptorSets(mDevice->getDevice(), &allocInfo, &mDescriptorSets[0]) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: Failed to allocate descriptor sets!");
        }

        for (int i = 0; i < frameCount; ++i)
        {
            std::vector<VkWriteDescriptorSet> descriptorSetWrites{};

            for (const auto& param : params)
            {
                VkWriteDescriptorSet descriptorSetWrite{};
                descriptorSetWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;  
                descriptorSetWrite.dstSet          = mDescriptorSets[i];                      // 目标描述符集（当前帧）
                descriptorSetWrite.dstArrayElement = 0;                                       // 数组起始索引
                descriptorSetWrite.descriptorType  = param->mDescriptorType;                  // 资源类型
                descriptorSetWrite.descriptorCount = param->mCount;                           // 描述符数量（数组大小）
                descriptorSetWrite.dstBinding      = param->mBinding;                         // 绑定点索引
                
                // 配置资源信息
                if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                {
                    // 当前帧对应的UBO缓冲区信息（双缓冲：第0帧用第0个缓冲区）
                    descriptorSetWrite.pBufferInfo = &param->mBuffers[i]->getBufferInfo();
                }

                if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                {
                    // 纹理信息（所有帧共享同一个纹理）
                    descriptorSetWrite.pImageInfo = &param->mTexture->getImageInfo();
                }

                // 添加到写操作列表
                descriptorSetWrites.push_back(descriptorSetWrite);
            }

            // 批量更新描述符集（一次提交所有写操作）
            vkUpdateDescriptorSets(mDevice->getDevice(),
                                   static_cast<uint32_t>(descriptorSetWrites.size()),  // 写操作数量
                                   descriptorSetWrites.data(),                         // 写操作数组
                                   0,                                                  // 复制操作数量（无）
                                   nullptr);                                           // 复制操作数组（无）
        }
    }

    DescriptorSet::~DescriptorSet() {}
}
