#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    /**
    * @class Buffer
    * @brief Vulkan缓冲区资源封装类
    *
    * 提供统一的缓冲区管理接口，支持：
    * - 顶点缓冲区
    * - 索引缓冲区
    * - 统一变量缓冲区
    * - 暂存缓冲区
    * 并实现两种数据更新机制：
    * 1. 直接内存映射 (HostVisible)
    * 2. 暂存缓冲区复制 (DeviceLocal)
    */
    class Buffer
    {
    
    public:
        using Ptr = std::shared_ptr<Buffer>;
        static Ptr create(
            const Device::Ptr& device,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties)
        {
            return std::make_shared<Buffer>(device, size, usage, properties);
        }

    public:

        /**
        * @brief 创建顶点缓冲区 (带初始数据)
        * @param device 关联设备
        * @param size   数据大小
        * @param pData  顶点数据指针
        */
        static Ptr createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, void * pData);

        /**
        * @brief 创建索引缓冲区 (带初始数据)
        * @param device 关联设备
        * @param size   数据大小
        * @param pData  索引数据指针
        */
        static Ptr createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData);

        /**
        * @brief 创建统一变量缓冲区
        * @param device 关联设备
        * @param size   缓冲区大小
        * @param pData  初始数据(可选)
        *
        * 默认创建为HostVisible|HostCoherent内存
        */
        static Ptr createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData = nullptr);

        /**
        * @brief 创建暂存缓冲区
        * @param device 关联设备
        * @param size   缓冲区大小
        * @param pData  初始数据(可选)
        *
        * 默认创建为HostVisible|HostCoherent内存
        */
        static Ptr createStageBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData = nullptr);

    public:

        /**
        * @brief 构造函数 - 创建Vulkan缓冲区
        *
        * @param device     关联设备
        * @param size       缓冲区大小
        * @param usage      缓冲区用途标志
        * @param properties 内存属性标志
        */
        Buffer(
            const Device::Ptr &device,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties
        );

        /// 析构函数 - 自动释放缓冲区资源
        ~Buffer();

        // 1 通过内存 Mapping 的形式，直接对内存进行更改，适用于 HostVisible 类型的内存
        // 2 如果本内存是 LocalOptimal，那么就必须创建中间的 StageBuffer，先复制到 StageBuffer，再拷贝入目标 Buffer

        /**
        * @brief 通过内存映射更新缓冲区数据
        *
        * @param data 源数据指针
        * @param size 数据大小
        *
        * 要求内存具有VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT属性
		* 如果 Buffer 被频繁更改，建议使用此方法。
        */
        void updateBufferByMap(void *data, size_t size);

        /**
        * @brief 通过暂存缓冲区更新数据
        *
        * @param data 源数据指针
        * @param size 数据大小
        *
        * 1. 创建临时暂存缓冲区
        * 2. 复制数据到暂存缓冲区
        * 3. 复制暂存缓冲区到目标缓冲区
        *
        * 适用于设备本地(DeviceLocal)内存
        */
        void updateBufferByStage(void* data, size_t size);

        /**
        * @brief 执行缓冲区间复制操作
        *
        * @param srcBuffer 源缓冲区
        * @param dstBuffer 目标缓冲区
        * @param size 复制数据大小
        */
        void copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);

        /// 获取底层VkBuffer句柄
        [[nodiscard]] auto getBuffer() const { return mBuffer; }

        /// 获取描述符缓冲区信息(用于描述符集绑定)
        [[nodiscard]] VkDescriptorBufferInfo& getBufferInfo() { return mBufferInfo; }
        
    private:
        /**
        * @brief 查找合适的内存类型索引
        *
        * @param typeFilter 内存类型筛选位
        * @param properties 需要的内存属性
        * @return uint32_t 内存类型索引
        */
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    private:
        VkBuffer mBuffer{ VK_NULL_HANDLE };              // Vulkan缓冲区句柄
        VkDeviceMemory mBufferMemory{ VK_NULL_HANDLE };  // 缓冲区内存对象
        Device::Ptr mDevice{ nullptr };                  // 关联设备
        VkDescriptorBufferInfo mBufferInfo{};            // 描述符绑定信息
    };
}