
#include "descriptorPool.h"

namespace LearnVulkan::Wrapper
{
    DescriptorPool::DescriptorPool(const Device::Ptr &device)
    {
        mDevice = device;
    }

    DescriptorPool::~DescriptorPool()
    {
        if (mPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(mDevice->getDevice(), mPool, nullptr);
        }
    } 

    void DescriptorPool::build(std::vector<UniformParameter::Ptr>& params, const int& frameCount)
    {
        // decriptor
        // descriptorSet(decriptorA(Buffer)，decriptorA，decriptorB)
        // descriptorSet * N 因为描述符集当中，绑定了buffer,当前一帧提交的时候，其他的帧正在绘制当中，
        // 即，uniformbuffer，可能正在被读取，此时cpu端的下一个循环，却对其进行了数据的修改

        /*
        * 设计目标：创建足够容纳多帧资源的描述符池
        * - 每帧都需要独立的描述符集，避免绘制时资源冲突
        * - 池大小需要包含所有绑定点类型乘以帧数
        */

        // 1. 统计不同资源类型的绑定点数量
        int uniformBufferCount = 0;  // UBO类型绑定点计数
        int textureCount       = 0;  // 纹理采样器类型绑定点计数

        for (const auto& param : params)
        {
            if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) { uniformBufferCount++; }
            if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) { textureCount++; }

            // 注：可扩展支持更多描述符类型
        }

        // 描述每一种uniform都有多少个
        // 2. 创建描述符池大小配置
        std::vector<VkDescriptorPoolSize> poolSizes{};  // 池容量配置

        // 配置UBO类型容量
        VkDescriptorPoolSize uniformBufferSize{};
        uniformBufferSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformBufferSize.descriptorCount = uniformBufferCount * frameCount;    // 总量 = 绑定点数 × 帧数
        poolSizes.push_back(uniformBufferSize);

        // 配置纹理采样器类型容量
        VkDescriptorPoolSize textureSize{};
        textureSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureSize.descriptorCount = textureCount * frameCount;                // 这边的size是指，有多少个descriptor
        poolSizes.push_back(textureSize);

        // 3. 创建描述符池
        VkDescriptorPoolCreateInfo createInfo{};
        createInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());     // 容量类型数量
        createInfo.pPoolSizes    = poolSizes.data();                            // 容量配置数组
        createInfo.maxSets       = static_cast<uint32_t>(frameCount);           // 最大描述符集数量（等于帧数）

        // 调用Vulkan API创建描述符池
        if (vkCreateDescriptorPool(mDevice->getDevice(), &createInfo, nullptr, &mPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create Descriptor pool");
        }
    }
}
