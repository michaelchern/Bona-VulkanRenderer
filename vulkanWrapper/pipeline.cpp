#include "pipeline.h"

namespace LearnVulkan::Wrapper
{
    // ���캯������ʼ������״̬�ṹ��
    Pipeline::Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass)
    {
        mDevice = device;                                                                       // �����߼��豸����
        mRenderPass = renderPass;                                                               // ������Ⱦͨ������

        // ��ʼ������״̬�ṹ��������ֶ�
        mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;    
        mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;     
        mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;           
        mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;        
        mSampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;          
        mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;           
        mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;  
        mLayoutState.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;                     
    }

    // �������������ٹ�����Դ
    Pipeline::~Pipeline()
    {
        // 1. ���ٹ��߲���
        if (mLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
        }

        // 2. ���ٹ��߶���
        if (mPipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
        }
    }

    // ������ɫ����
    void Pipeline::setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup)
    {
        mShaders = shaderGroup;
    }

    // ����ͼ�ι���
    void Pipeline::build()
    {
        // 1. ׼����ɫ���׶δ�����Ϣ
        std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
        for (const auto& shader : mShaders)
        {
            VkPipelineShaderStageCreateInfo shaderCreateInfo{};
            shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderCreateInfo.stage = shader->getShaderStage();                             // ��ɫ���׶Σ�����/Ƭ�εȣ�
            shaderCreateInfo.pName = shader->getShaderEntryPoint().c_str();                // ��ں�����
            shaderCreateInfo.module = shader->getShaderModule();                           // ��ɫ��ģ��

            shaderCreateInfos.push_back(shaderCreateInfo);
        }

        // 2. �����ӿںͲü�״̬
        mViewportState.viewportCount = static_cast<uint32_t>(mViewports.size());
        mViewportState.pViewports = mViewports.data();                            // �ӿ�����
        mViewportState.scissorCount = static_cast<uint32_t>(mScissors.size());
        mViewportState.pScissors = mScissors.data();                              // �ü���������

        // 3. ������ɫ���״̬
        mBlendState.attachmentCount = static_cast<uint32_t>(mBlendAttachmentStates.size());  
        mBlendState.pAttachments = mBlendAttachmentStates.data();                            // ÿ�������Ļ������

        // 4. �������߲��֣�����Ѵ����������٣�
        if (mLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
        }

        // �����µĹ��߲��֣����������������ͳ�����
        if (vkCreatePipelineLayout(mDevice->getDevice(), &mLayoutState, nullptr, &mLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create pipeline layout");
        }

        // 5. ��д���ߴ�����Ϣ
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        // 5.1 ��ɫ���׶�
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderCreateInfos.size());
        pipelineCreateInfo.pStages = shaderCreateInfos.data();

        // 5.2 �̶�����״̬
        pipelineCreateInfo.pVertexInputState = &mVertexInputState;     // ��������
        pipelineCreateInfo.pInputAssemblyState = &mAssemblyState;      // ͼԪװ��
        pipelineCreateInfo.pViewportState = &mViewportState;           // �ӿڲü�
        pipelineCreateInfo.pRasterizationState = &mRasterState;        // ��դ��
        pipelineCreateInfo.pMultisampleState = &mSampleState;          // ���ز���
        pipelineCreateInfo.pDepthStencilState = nullptr;               // ���/ģ�壨��δʵ�֣�
        pipelineCreateInfo.pColorBlendState = &mBlendState;            // ��ɫ���
        pipelineCreateInfo.layout = mLayout;                           // ���߲���

        // 5.3 ��Ⱦͨ������ͨ��
        pipelineCreateInfo.renderPass = mRenderPass->getRenderPass();  // ������Ⱦͨ��
        pipelineCreateInfo.subpass = 0;                                // ʹ�õ�һ����ͨ��

        // 5.4 ���߼̳У������Ż���
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;        // �޻�������
        pipelineCreateInfo.basePipelineIndex = -1;                     // �޻�����������

        // 6. ���پɹ��ߣ�������ڣ�
        if (mPipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
        }

        // 7. ����ͼ�ι���
        // ����˵����
        // - VK_NULL_HANDLE: ��ʹ�ù��߻���
        // - 1: ����1������
        // - &pipelineCreateInfo: ������Ϣ
        // - nullptr: ��ʹ�÷���ص�
        // - &mPipeline: ������߾��
        if (vkCreateGraphicsPipelines(mDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Error:failed to create pipeline");
        }
    }
}