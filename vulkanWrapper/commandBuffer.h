#pragma once

#include "base.h"
#include "commandPool.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    /**
     * @brief Vulkan命令缓冲区的封装类
     *
     * 负责记录、提交和执行GPU命令的包装类。
     * 支持主命令缓冲区和二级命令缓冲区两种模式。
     */
    class CommandBuffer
    {
    public:

        using Ptr = std::shared_ptr<CommandBuffer>;

        /**
         * @brief 创建命令缓冲区的工厂方法
         *
         * @param device 关联的逻辑设备
         * @param commandPool 分配此命令缓冲区的命令池
         * @param asSecondary 是否创建为二级命令缓冲区（默认为主命令缓冲区）
         */
        static Ptr create(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary = false)
        { 
            return std::make_shared<CommandBuffer>(device, commandPool, asSecondary); 
        }

        /// 构造函数（在命令池中分配命令缓冲区）
        CommandBuffer(const Device::Ptr &device, const CommandPool::Ptr &commandPool, bool asSecondary = false);

        /// 析构函数（不需要显式销毁，由命令池统一管理）
        ~CommandBuffer();

        // ================================================
        // 命令记录接口（必须按顺序调用）
        //begin -> xxx -> beginRenderPass -> xxxx 绑定各类实际数据 -> endRenderPass -> end
        // ================================================

        /**
         * @brief 开始记录命令
         *
         * @param flag 使用标志（控制命令缓冲区行为）：
         *   - VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: 这个命令，只会被使用提交一次
         *   - VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: 这个命令缓冲，是一个二级缓冲，位于一个renderPass当中
         *   - VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: 命令已经被提交了，执行期间，可以再次提交
         * @param inheritance 如果本命令缓冲是二级缓冲，那么这个结构体，记录了他所属的主命令信息/继承信息
         */
        void begin(VkCommandBufferUsageFlags flag = 0, const VkCommandBufferInheritanceInfo& inheritance = {});

        /**
         * @brief 开始渲染通道
         *
         * @param renderPassBeginInfo 渲染通道起始信息
         * @param subPassContents 子通道内容类型：
         *   - VK_SUBPASS_CONTENTS_INLINE: 渲染指令会被记录在命令缓冲，本命令缓冲肯定就是主命令缓冲
         *   - VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: 渲染指令放在了二级指令缓冲当中,适用于主命令缓冲调用 beginRenderPass 的时候且使用了二级命令缓冲的情况下，使用
         */
        void beginRenderPass(const VkRenderPassBeginInfo &renderPassBeginInfo, const VkSubpassContents &subPassContents = VK_SUBPASS_CONTENTS_INLINE);

        /// 绑定图形管线
        void bindGraphicPipeline(const VkPipeline &pipeline);

        /// 绑定顶点缓冲区（支持多个缓冲区）
        void bindVertexBuffer(const std::vector<VkBuffer> &buffers);

        /// 绑定索引缓冲区
        void bindIndexBuffer(const VkBuffer& buffer);

        /// 绑定描述符集（着色器资源）
        void bindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet& descriptorSet);

        /// 执行非索引绘制
        void draw(size_t vertexCount);

        /// 执行索引绘制
        void drawIndex(size_t indexCount);

        /// 结束渲染通道
        void endRenderPass();

        /// 结束命令记录
        void end();

        // ================================================
        // 实用命令操作
        // ================================================

        /**
         * @brief 复制缓冲区数据
         *
         * @param srcBuffer 源缓冲区
         * @param dstBuffer 目标缓冲区
         * @param copyInfoCount 复制操作数量
         * @param copyInfos 复制操作详情
         */
        void copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyInfos);

        /**
         * @brief 从缓冲区复制数据到图像
         *
         * @param srcBuffer 源缓冲区
         * @param dstImage 目标图像
         * @param dstImageLayout 目标图像布局
         * @param width 图像宽度
         * @param height 图像高度
         */
        void copyBufferToImage(VkBuffer srcBuffer,  VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height);

        /// 转换图像布局（内存屏障）
        void transferImageLayout(const VkImageMemoryBarrier& imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

        // ================================================
        // 提交与执行
        // ================================================

        /**
         * @brief 同步提交到队列（阻塞等待完成）
         *
         * @param queue 目标队列
         * @param fence 栅栏对象（可选，用于CPU-GPU同步）
         */
        void submitSync(VkQueue queue, VkFence fence = VK_NULL_HANDLE);

        /// 获取底层VkCommandBuffer句柄
        [[nodiscard]] auto getCommandBuffer() const { return mCommandBuffer; }

    private:
        VkCommandBuffer  mCommandBuffer{ VK_NULL_HANDLE };  // Vulkan命令缓冲区句柄
        Device::Ptr      mDevice{ nullptr };                // 关联设备
        CommandPool::Ptr mCommandPool{ nullptr };           // 来源命令池
    };
}