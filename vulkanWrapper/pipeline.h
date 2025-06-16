#pragma once

#include "base.h"
#include "device.h"
#include "shader.h"
#include "renderPass.h"

namespace LearnVulkan::Wrapper
{
    // Vulkan 图形管线管理类
    class Pipeline
    {
    public:
        using Ptr = std::shared_ptr<Pipeline>;
        static Ptr create(const Device::Ptr& device, const RenderPass::Ptr& renderPass)
        {
            return std::make_shared<Pipeline>(device, renderPass);
        }

        // 构造函数：初始化管线
        Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass);

        // 析构函数：销毁管线资源
        ~Pipeline();

        // 设置着色器组（顶点/片段等）
        void setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup);

        // 设置视口参数（可设置多个视口）
        void setViewports(const std::vector<VkViewport>& viewports) { mViewports = viewports; }

        // 设置裁剪区域（可设置多个裁剪区域）
        void setScissors(const std::vector<VkRect2D>& scissors) { mScissors = scissors; }

        // 添加混合附件状态（用于颜色混合）
        void pushBlendAttachment(const VkPipelineColorBlendAttachmentState& blendAttachment)
        {
            mBlendAttachmentStates.push_back(blendAttachment);
        }

        // 构建图形管线（完成所有配置后调用）
        void build();

    public:
        // === 管线状态配置 ===
        VkPipelineVertexInputStateCreateInfo mVertexInputState{};                   // 顶点输入状态
        VkPipelineInputAssemblyStateCreateInfo mAssemblyState{};                    // 图元装配状态
        VkPipelineViewportStateCreateInfo mViewportState{};                         // 视口状态
        VkPipelineRasterizationStateCreateInfo mRasterState{};                      // 光栅化状态
        VkPipelineMultisampleStateCreateInfo mSampleState{};                        // 多重采样状态
        std::vector<VkPipelineColorBlendAttachmentState> mBlendAttachmentStates{};  // 颜色混合附件状态
        VkPipelineColorBlendStateCreateInfo mBlendState{};                          // 全局混合状态
        VkPipelineDepthStencilStateCreateInfo mDepthStencilState{};                 // 深度/模板测试状态
        VkPipelineLayoutCreateInfo mLayoutState{};                                  // 管线布局状态

    public:
        // === 访问方法 ===
        [[nodiscard]] auto getPipeline() const { return mPipeline; }                // 获取管线对象
        [[nodiscard]] auto getLayout() const { return mLayout; }                    // 获取管线布局

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