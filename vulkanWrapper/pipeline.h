#pragma once

#include "base.h"
#include "device.h"
#include "shader.h"
#include "renderPass.h"

namespace LearnVulkan::Wrapper
{
    /**
     * @class Pipeline
     * @brief Vulkan 图形管线管理类（封装完整的图形管线生命周期）
     *
     * 核心功能：
     *  1. 配置和管理图形管线的所有状态（顶点输入/图元装配/着色器等）
     *  2. 简化复杂管线创建流程
     *  3. 自动管理管线对象和管线布局的生命周期
     *  4. 支持多视口、多裁剪区域等高级特性
     */
    class Pipeline
    {
    public:
        using Ptr = std::shared_ptr<Pipeline>;

        /**
         * @brief 创建Pipeline对象的工厂方法
         * @param device 关联的Vulkan逻辑设备
         * @param renderPass 管线要使用的渲染通道
         * @return 指向新Pipeline对象的共享指针
         */
        static Ptr create(const Device::Ptr& device, const RenderPass::Ptr& renderPass)
        {
            return std::make_shared<Pipeline>(device, renderPass);
        }

        // 构造函数：初始化管线基础状态
        Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass);

        // 析构函数：自动销毁Vulkan资源
        ~Pipeline();

        // 设置着色器组（顶点/片段等）
        void setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup);

        // 设置视口参数（支持多视口配置）
        void setViewports(const std::vector<VkViewport>& viewports) { mViewports = viewports; }

        // 设置裁剪区域（支持多裁剪区域配置）
        void setScissors(const std::vector<VkRect2D>& scissors) { mScissors = scissors; }

        // 添加颜色混合附件状态（每个附件一个混合配置）
        void pushBlendAttachment(const VkPipelineColorBlendAttachmentState& blendAttachment)
        {
            mBlendAttachmentStates.push_back(blendAttachment);
        }

        // 构建图形管线（完成所有配置后调用）
        void build();

    public:
        // === 公开的管线状态配置区域 ===
        // 注意：以下状态必须在调用build()前配置

        VkPipelineVertexInputStateCreateInfo mVertexInputState{};                   // 顶点输入状态（绑定和属性描述）
        VkPipelineInputAssemblyStateCreateInfo mAssemblyState{};                    // 图元装配状态（拓扑结构等）
        VkPipelineViewportStateCreateInfo mViewportState{};                         // 视口状态（配置视口和裁剪区）
        VkPipelineRasterizationStateCreateInfo mRasterState{};                      // 光栅化状态（多边形模式/剔除等）
        VkPipelineMultisampleStateCreateInfo mSampleState{};                        // 多重采样状态（抗锯齿设置）
        std::vector<VkPipelineColorBlendAttachmentState> mBlendAttachmentStates{};  // 每个颜色附件的混合状态
        VkPipelineColorBlendStateCreateInfo mBlendState{};                          // 全局颜色混合状态
        VkPipelineDepthStencilStateCreateInfo mDepthStencilState{};                 // 深度/模板测试状态
        VkPipelineLayoutCreateInfo mLayoutState{};                                  // 管线布局配置（描述符集/推送常量）

    public:
        // === 访问方法 ===
        [[nodiscard]] auto getPipeline() const { return mPipeline; }                // 获取VkPipeline管线对象
        [[nodiscard]] auto getLayout() const { return mLayout; }                    // 获取VkPipelineLayout布局对象

    private:
        VkPipeline mPipeline{ VK_NULL_HANDLE };      // Vulkan 管线对象句柄
        VkPipelineLayout mLayout{ VK_NULL_HANDLE };  // Vulkan 管线布局句柄
        Device::Ptr mDevice{ nullptr };              // 逻辑设备引用
        RenderPass::Ptr mRenderPass{ nullptr };      // 渲染通道引用

        std::vector<Shader::Ptr> mShaders{};         // 着色器对象集合
        std::vector<VkViewport> mViewports{};        // 视口配置列表
        std::vector<VkRect2D> mScissors{};           // 裁剪区域列表
    };
}