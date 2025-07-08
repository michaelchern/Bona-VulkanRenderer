#include "pipeline.h"

namespace LearnVulkan::Wrapper
{
    /**
     * @brief 管线构造函数
     *
     * 初始化：
     *  1. 保存关联的逻辑设备和渲染通道
     *  2. 设置所有状态结构体的 sType 字段（Vulkan API 要求）
     */
    Pipeline::Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass)
    {
        mDevice     = device;      // 保存逻辑设备（智能指针）
        mRenderPass = renderPass;  // 保存渲染通道（智能指针）

        // 为所有管线状态结构体设置标准类型标识符
        mVertexInputState.sType  = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        mAssemblyState.sType     = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        mViewportState.sType     = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        mRasterState.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        mSampleState.sType       = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        mBlendState.sType        = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        mLayoutState.sType       = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    }

    /**
     * @brief 管线析构函数
     *
     * 遵循 RAII 原则释放资源：
     *  1. 销毁管线布局（如果存在）
     *  2. 销毁管线对象（如果存在）
     */
    Pipeline::~Pipeline()
    {
        // 销毁管线布局（描述符集布局/推送常量的容器）
        if (mLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
        }

        // 销毁图形管线对象
        if (mPipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
        }
    }

    /**
     * @brief 设置着色器组
     *
     * @param shaderGroup 包含所有阶段着色器的向量（顶点/片段/几何等）
     */
    void Pipeline::setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup)
    {
        mShaders = shaderGroup;  // 保存着色器对象列表
    }

    /**
     * @brief 构建图形管线（核心函数）
     *
     * 执行步骤：
     *  1. 准备着色器阶段信息
     *  2. 配置视口和裁剪区域
     *  3. 设置颜色混合状态
     *  4. 创建/重建管线布局
     *  5. 填写管线创建信息
     *  6. 创建图形管线对象
     *
     * @throws std::runtime_error 创建失败时抛出异常
     */
    void Pipeline::build()
    {
        // ===== 1. 准备着色器阶段创建信息 =====
        std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
        for (const auto& shader : mShaders)
        {
            VkPipelineShaderStageCreateInfo shaderCreateInfo{};
            shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderCreateInfo.stage = shader->getShaderStage();                             // 着色器阶段标志
            shaderCreateInfo.pName = shader->getShaderEntryPoint().c_str();                // 入口函数名
            shaderCreateInfo.module = shader->getShaderModule();                           // 着色器模块句柄

            shaderCreateInfos.push_back(shaderCreateInfo);
        }

        // ===== 2. 配置视口和裁剪状态 =====
        // 设置视口参数（应提前通过 setViewports() 设置）
        mViewportState.viewportCount = static_cast<uint32_t>(mViewports.size());
        mViewportState.pViewports = mViewports.data();                            // 指向视口配置数组
        // 设置裁剪区域（应提前通过 setScissors() 设置）
        mViewportState.scissorCount = static_cast<uint32_t>(mScissors.size());
        mViewportState.pScissors = mScissors.data();                              // 指向裁剪区域数组

        // ===== 3. 配置颜色混合状态 =====
        // 设置每个颜色附件的混合状态（应提前通过 addBlendAttachment() 添加）
        mBlendState.attachmentCount = static_cast<uint32_t>(mBlendAttachmentStates.size());  
        mBlendState.pAttachments = mBlendAttachmentStates.data();                            // 附件混合状态数组

        // ===== 4. 创建管线布局 =====
        // 先销毁旧布局（如果重建管线）
        if (mLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
        }

        // 创建新的管线布局（包含描述符集布局和推送常量信息）
        if (vkCreatePipelineLayout(mDevice->getDevice(), &mLayoutState, nullptr, &mLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create pipeline layout");
        }

        // ===== 5. 填写管线创建信息结构体 =====
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        // -- 着色器阶段 --
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderCreateInfos.size());
        pipelineCreateInfo.pStages = shaderCreateInfos.data();

        // -- 固定功能状态 --
        pipelineCreateInfo.pVertexInputState = &mVertexInputState;     // 顶点输入格式
        pipelineCreateInfo.pInputAssemblyState = &mAssemblyState;      // 图元类型（三角形/线等）
        pipelineCreateInfo.pViewportState = &mViewportState;           // 视口和裁剪区
        pipelineCreateInfo.pRasterizationState = &mRasterState;        // 光栅化设置
        pipelineCreateInfo.pMultisampleState = &mSampleState;          // 多重采样设置
        pipelineCreateInfo.pDepthStencilState = nullptr;               // 深度模板（当前未实现）
        pipelineCreateInfo.pColorBlendState = &mBlendState;            // 颜色混合状态
        pipelineCreateInfo.pDynamicState = nullptr;                    // 动态状态（当前未实现）

        // -- 管线布局 --
        pipelineCreateInfo.layout = mLayout;                           // 关联管线布局

        // -- 渲染通道 --
        pipelineCreateInfo.renderPass = mRenderPass->getRenderPass();  // 关联渲染通道
        pipelineCreateInfo.subpass = 0;                                // 使用渲染通道的第一个子通道

        // -- 管线继承（优化）--
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;        // 无基础管线
        pipelineCreateInfo.basePipelineIndex = -1;                     // 无基础管线索引

        // ===== 6. 销毁旧管线（如果重建）=====
        if (mPipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
        }

        // ===== 7. 创建图形管线 =====
        // 参数说明：
        // device - 使用的逻辑设备
        // VK_NULL_HANDLE - 不使用管线缓存
        // 1 - 创建的管线数量
        // &pipelineCreateInfo - 创建信息数组
        // nullptr - 自定义分配器
        // &mPipeline - 输出的管线句柄
        if (vkCreateGraphicsPipelines(mDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Error:failed to create pipeline");
        }
    }
}