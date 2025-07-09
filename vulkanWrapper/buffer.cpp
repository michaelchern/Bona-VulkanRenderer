
#include "buffer.h"
#include "commandBuffer.h"
#include"commandPool.h"

namespace LearnVulkan::Wrapper
{
    /**
    * @brief 创建顶点缓冲区（带初始数据）
    *
    * 1. 创建设备本地(DEVICE_LOCAL)的顶点缓冲区
    * 2. 通过临时暂存缓冲区上传初始数据
    *
    * @param device       关联设备
    * @param size         数据大小(字节)
    * @param pData        顶点数据指针
    * @return Buffer::Ptr 顶点缓冲区智能指针
    */
    Buffer::Ptr Buffer::createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData)
    {
        // 创建设备本地缓冲区 (GPU专用内存)
        auto buffer = Buffer::create(
            device, size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |   // 可作为传输目标
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,   // 顶点缓冲区用途
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT  // 设备本地内存(高性能)，GPU本地才可读
        );

        // 通过暂存缓冲区上传数据
        buffer->updateBufferByStage(pData, size);

        return buffer;
    }

    /**
    * @brief 创建索引缓冲区（带初始数据）
    *
    * @param device 关联设备
    * @param size 数据大小(字节)
    * @param pData 索引数据指针
    * @return Buffer::Ptr 索引缓冲区智能指针
    */
    Buffer::Ptr Buffer::createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData)
    {
        // 创建设备本地缓冲区
        auto buffer = Buffer::create(
            device, size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |   // 传输目标
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,    // 索引缓冲区用途
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT  
        );

        // 通过暂存缓冲区上传数据
        buffer->updateBufferByStage(pData, size);

        return buffer;
    }

    /**
    * @brief 创建统一变量缓冲区
    *
    * 使用主机可见(HOST_VISIBLE)内存，便于CPU频繁更新
    *
    * @param device 关联设备
    * @param size 缓冲区大小
    * @param pData 初始数据(可选)
    * @return Buffer::Ptr 统一变量缓冲区智能指针
    */
    Buffer::Ptr Buffer::createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData)
    {
        // 创建主机可见/一致的缓冲区
        auto buffer = Buffer::create(
            device, size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,    // 统一变量缓冲区用途
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |  // CPU可访问
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT   // 自动保持CPU/GPU内存一致性
        );

        // 如果有初始数据，直接映射上传
        if (pData != nullptr)
        {
            buffer->updateBufferByStage(pData, size);
        }

        return buffer;
    }

    /**
    * @brief 创建暂存缓冲区
    *
    * 用于数据传输的临时缓冲区（主机可见）
    *
    * @param device 关联设备
    * @param size 缓冲区大小
    * @param pData 初始数据(可选)
    * @return Buffer::Ptr 暂存缓冲区智能指针
    */
    Buffer::Ptr Buffer::createStageBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData)
    {
        // 创建传输源(TRANSFER_SRC)缓冲区
        auto buffer = Buffer::create(
            device, size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,      // 仅作为传输源
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |  
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT   
        );

        // 如果有初始数据，直接映射上传
        if (pData != nullptr)
        {
            buffer->updateBufferByMap(pData, size);
        }

        return buffer;
    }

    /**
    * @brief 缓冲区构造函数
    *
    * 创建Vulkan缓冲区对象并分配内存
    *
    * @param device 关联设备
    * @param size 缓冲区大小
    * @param usage 缓冲区用途标志
    * @param properties 内存属性标志
    */
    Buffer::Buffer(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        mDevice = device;  // 存储关联设备

        // 配置缓冲区创建信息
        VkBufferCreateInfo createInfo{};
        createInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.size        = size;                       // 缓冲区大小
        createInfo.usage       = usage;                      // 用途标志
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // 独占访问模式

		// 创建 Vulkan 缓冲区对象，一般在 CPU 端进行内存的创建，创建的对象是 CPU 端的描述符，是一个描述对象，没有在 GPU 上创建实际的内存对象
        if (vkCreateBuffer(mDevice->getDevice(), &createInfo, nullptr, &mBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: Failed to create buffer!");
        }

        // 获取内存需求（类型/大小/对齐等）
        VkMemoryRequirements memReq{};
        vkGetBufferMemoryRequirements(mDevice->getDevice(), mBuffer, &memReq);

        // 配置内存分配信息
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;

        // 符合我上述 buffer 需求的内存类型的 ID 们！0x001 0x010
        // 查找符合要求的内存类型索引
        allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

        // 分配设备内存
        if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mBufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to allocate memory");
        }

        // 将缓冲区与内存绑定（偏移为0）
        vkBindBufferMemory(mDevice->getDevice(), mBuffer, mBufferMemory, 0);

        // 初始化描述符缓冲区信息
        mBufferInfo.buffer = mBuffer;
        mBufferInfo.offset = 0;
        mBufferInfo.range  = size;
    }

    Buffer::~Buffer()
    {
        if (mBuffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(mDevice->getDevice(), mBuffer, nullptr);
        }

        if (mBufferMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(mDevice->getDevice(), mBufferMemory, nullptr);
        }
    }

    /**
    * @brief 查找合适的内存类型
    *
    * 根据内存需求属性和所需属性寻找最佳内存类型
    *
    * @param typeFilter 内存类型筛选位掩码
    * @param properties 需要的内存属性标志
    * @return uint32_t  找到的内存类型索引
    */
    uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        // 获取物理设备内存属性
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(mDevice->getPhysicalDevice(), &memProps);

        // 0x001 | 0x100 = 0x101  i = 0 第i个对应类型就是  1 << i 1   i = 1 0x010
        // 遍历所有内存类型
        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
        {
            // 检查：1) 内存类型是否在筛选掩码中 2) 是否包含所需属性
            if ((typeFilter & (1 << i)) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties))
            {
                return i;  // 返回匹配的内存类型索引
            }
        }

        throw std::runtime_error("Error: Cannot find the property memory type!");
    }

    /**
    * @brief 通过内存映射更新缓冲区数据
    *
    * 适用于主机可见(HOST_VISIBLE)内存
    *
    * @param data 源数据指针
    * @param size 数据大小
    */
    void Buffer::updateBufferByMap(void* data, size_t size)
    {
        void* memPtr = nullptr;

        // 映射设备内存到主机地址空间
        vkMapMemory(mDevice->getDevice(), mBufferMemory, 0, size, 0, &memPtr);

        // 将数据复制到映射的内存区域
        memcpy(memPtr, data, size);

        // 解除内存映射
        vkUnmapMemory(mDevice->getDevice(), mBufferMemory);
    }

    /**
    * @brief 通过暂存缓冲区更新数据
    *
    * 1. 创建主机可见的暂存缓冲区
    * 2. 将数据复制到暂存缓冲区
    * 3. 将暂存缓冲区数据复制到目标缓冲区
    *
    * 适用于设备本地(DEVICE_LOCAL)内存
    *
    * @param data 源数据指针
    * @param size 数据大小
    */
    void Buffer::updateBufferByStage(void* data, size_t size)
    {
        // 1. 创建主机可见的暂存缓冲区
        auto stageBuffer = Buffer::create(mDevice,
                                          size,
                                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // 2. 将数据复制到暂存缓冲区
        stageBuffer->updateBufferByMap(data, size);

        // 3. 复制数据到目标设备本地缓冲区
        copyBuffer(stageBuffer->getBuffer(), mBuffer, static_cast<VkDeviceSize>(size));

        // 注意：stageBuffer 离开作用域后自动销毁
    }

    /**
    * @brief 执行缓冲区间复制
    *
    * 使用一次性命令缓冲区进行数据传输
    *
    * @param srcBuffer 源缓冲区句柄
    * @param dstBuffer 目标缓冲区句柄
    * @param size      复制数据大小
    */
    void Buffer::copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size)
    {
        // 创建临时命令池和命令缓冲区
        auto commandPool   = CommandPool::create(mDevice);
        auto commandBuffer = CommandBuffer::create(mDevice, commandPool);

        // 开始记录一次性命令
        commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // 配置复制区域（整个缓冲区）
        VkBufferCopy copyInfo{};
        copyInfo.size = size;
        
        // 添加复制命令
        commandBuffer->copyBufferToBuffer(srcBuffer,
                                          dstBuffer,
                                          1,              // 复制操作数量
                                          { copyInfo });  // 复制区域数组

        // 结束命令记录
        commandBuffer->end();

        // 同步提交并等待完成
        commandBuffer->submitSync(mDevice->getGraphicQueue(), VK_NULL_HANDLE);
    }
}