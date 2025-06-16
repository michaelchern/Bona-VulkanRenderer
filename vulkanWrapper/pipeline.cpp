#include "pipeline.h"

namespace LearnVulkan::Wrapper
{
    // 构造函数：初始化管线状态结构体
    Pipeline::Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass)
    {
        mDevice = device;                                                                       // 保存逻辑设备引用
        mRenderPass = renderPass;                                                               // 保存渲染通道引用

        // 初始化所有状态结构体的类型字段
        mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;    
        mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;     
        mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;           
        mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;        
        mSampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;          
        mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;           
        mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;  
        mLayoutState.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;                     
    }

    // 析构函数：销毁管线资源
    Pipeline::~Pipeline()
    {
        // 1. 销毁管线布局
        if (mLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
        }

        // 2. 销毁管线对象
        if (mPipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
        }
    }

    // 设置着色器组
    void Pipeline::setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup)
    {
        mShaders = shaderGroup;
    }

    // 构建图形管线
    void Pipeline::build()
    {
        // 1. 准备着色器阶段创建信息
        std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
        for (const auto& shader : mShaders)
        {
            VkPipelineShaderStageCreateInfo shaderCreateInfo{};
            shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderCreateInfo.stage = shader->getShaderStage();                             // 着色器阶段（顶点/片段等）
            shaderCreateInfo.pName = shader->getShaderEntryPoint().c_str();                // 入口函数名
            shaderCreateInfo.module = shader->getShaderModule();                           // 着色器模块

            shaderCreateInfos.push_back(shaderCreateInfo);
        }

        // 2. 配置视口和裁剪状态
        mViewportState.viewportCount = static_cast<uint32_t>(mViewports.size());
        mViewportState.pViewports = mViewports.data();                            // 视口配置
        mViewportState.scissorCount = static_cast<uint32_t>(mScissors.size());
        mViewportState.pScissors = mScissors.data();                              // 裁剪区域配置

        // 3. 配置颜色混合状态
        mBlendState.attachmentCount = static_cast<uint32_t>(mBlendAttachmentStates.size());  
        mBlendState.pAttachments = mBlendAttachmentStates.data();                            // 每个附件的混合设置

        // 4. 创建管线布局（如果已存在则先销毁）
        if (mLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
        }

        // 创建新的管线布局（管理描述符和推送常量）
        if (vkCreatePipelineLayout(mDevice->getDevice(), &mLayoutState, nullptr, &mLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create pipeline layout");
        }

        // 5. 填写管线创建信息
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        // 5.1 着色器阶段
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderCreateInfos.size());
        pipelineCreateInfo.pStages = shaderCreateInfos.data();

        // 5.2 固定功能状态
        pipelineCreateInfo.pVertexInputState = &mVertexInputState;     // 顶点输入
        pipelineCreateInfo.pInputAssemblyState = &mAssemblyState;      // 图元装配
        pipelineCreateInfo.pViewportState = &mViewportState;           // 视口裁剪
        pipelineCreateInfo.pRasterizationState = &mRasterState;        // 光栅化
        pipelineCreateInfo.pMultisampleState = &mSampleState;          // 多重采样
        pipelineCreateInfo.pDepthStencilState = nullptr;               // 深度/模板（暂未实现）
        pipelineCreateInfo.pColorBlendState = &mBlendState;            // 颜色混合
        pipelineCreateInfo.layout = mLayout;                           // 管线布局

        // 5.3 渲染通道和子通道
        pipelineCreateInfo.renderPass = mRenderPass->getRenderPass();  // 关联渲染通道
        pipelineCreateInfo.subpass = 0;                                // 使用第一个子通道

        // 5.4 管线继承（用于优化）
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;        // 无基础管线
        pipelineCreateInfo.basePipelineIndex = -1;                     // 无基础管线索引

        // 6. 销毁旧管线（如果存在）
        if (mPipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
        }

        // 7. 创建图形管线
        // 参数说明：
        // - VK_NULL_HANDLE: 不使用管线缓存
        // - 1: 创建1个管线
        // - &pipelineCreateInfo: 创建信息
        // - nullptr: 不使用分配回调
        // - &mPipeline: 输出管线句柄
        if (vkCreateGraphicsPipelines(mDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Error:failed to create pipeline");
        }
    }
}