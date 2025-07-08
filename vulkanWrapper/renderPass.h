#pragma once

#include "base.h"
#include "device.h"

namespace LearnVulkan::Wrapper
{
    // 设计思路说明：
    /*
     * Vulkan 渲染通道（Render Pass）由以下核心组件构成：
     * 1. 附件描述（Attachment Description）
     *    - 描述颜色/深度/模板附件的格式、布局等属性
     *    - VkAttachmentDescription: 不是实际的附件，而是其规范描述
     *
     * 2. 附件引用（Attachment Reference）
     *    - 指定子通道使用的附件
     *    - VkAttachmentReference: 关联附件索引和期望布局
     *
     * 3. 子通道描述（Subpass Description）
     *    - 定义子通道的行为和依赖关系
     *    - VkSubpassDescription: 包含颜色/输入/深度附件引用
     *
     * 4. 子通道依赖（Subpass Dependency）
     *    - 描述不同子通道或主通道之间的同步关系
     *    - VkSubpassDependency: 管理执行顺序和内存访问
     */

    /**
     * @class SubPass
     * @brief 封装Vulkan子通道（Subpass）的创建和配置
     */
    class SubPass
    {
    public:
        SubPass();   // 构造函数

        ~SubPass();  // 析构函数

        /**
         * @brief 添加颜色附件引用
         * @param ref 颜色附件引用结构体
         */
        void addColorAttachmentReference(const VkAttachmentReference& ref);

        /**
         * @brief 添加输入附件引用
         * @param ref 输入附件引用结构体
         *
         * 输入附件用于从上一子通道读取数据
         */
        void addInputAttachmentReference(const VkAttachmentReference& ref);

        /**
         * @brief 设置深度模板附件引用
         * @param ref 深度/模板附件引用结构体
         *
         * 每个子通道最多只能有一个深度/模板附件
         */
        void setDepthStencilAttachmentReference(const VkAttachmentReference& ref);

        /**
         * @brief 构建子通道描述结构体
         *
         * 必须调用此方法后getSubPassDescription()才有效
         */
        void buildSubPassDescription();

        // 获取已构建的子通道描述结构体
        [[nodiscard]] auto getSubPassDescription() const { return mSubPassDescription; }

    private:
        VkSubpassDescription               mSubPassDescription{};               // Vulkan子通道描述
        std::vector<VkAttachmentReference> mColorAttachmentReferences{};        // 颜色附件引用集合
        std::vector<VkAttachmentReference> mInputAttachmentReferences{};        // 输入附件引用集合
        VkAttachmentReference              mDepthStencilAttachmentReference{};  // 深度/模板附件引用
    };

    /**
     * @class RenderPass
     * @brief 封装Vulkan渲染通道（Render Pass）的完整生命周期
     */
    class RenderPass
    {
    public:
        using Ptr = std::shared_ptr<RenderPass>;

        static Ptr create(const Device::Ptr& device) { return std::make_shared<RenderPass>(device); }

        RenderPass(const Device::Ptr& device);  // 构造函数

        ~RenderPass();                          // 析构函数

        /**
         * @brief 添加子通道到渲染通道
         * @param subpass 要添加的SubPass对象
         */
        void addSubPass(const SubPass& subpass);

        /**
         * @brief 添加子通道依赖
         * @param dependency 依赖关系描述结构体
         *
         * 用于控制子通道间的执行顺序和资源访问
         */
        void addDependency(const VkSubpassDependency& dependency);

        /**
         * @brief 添加附件描述
         * @param attachmentDes 附件描述结构体
         *
         * 描述附件的格式、加载操作等属性
         */
        void addAttachment(const VkAttachmentDescription& attachmentDes);

        /**
         * @brief 构建Vulkan渲染通道对象
         *
         * 在所有配置完成后调用此方法创建实际对象
         */
        void buildRenderPass();

        // 获取底层的VkRenderPass句柄
        [[nodiscard]] auto getRenderPass() const { return mRenderPass; }

    private:
        VkRenderPass                         mRenderPass{ VK_NULL_HANDLE };  // Vulkan渲染通道句柄

        std::vector<SubPass>                 mSubPasses{};                   // 子通道集合
        std::vector<VkSubpassDependency>     mDependencies{};                // 依赖关系集合
        std::vector<VkAttachmentDescription> mAttachmentDescriptions{};      // 附件描述集合

        Device::Ptr                          mDevice{ nullptr };             // 关联的逻辑设备
    };
}