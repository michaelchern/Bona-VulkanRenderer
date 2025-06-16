#pragma once

#include "base.h"
#include "device.h"
#include "shader.h"
#include "renderPass.h"

namespace LearnVulkan::Wrapper
{
    // Vulkan ͼ�ι��߹�����
    class Pipeline
    {
    public:
        using Ptr = std::shared_ptr<Pipeline>;
        static Ptr create(const Device::Ptr& device, const RenderPass::Ptr& renderPass)
        {
            return std::make_shared<Pipeline>(device, renderPass);
        }

        // ���캯������ʼ������
        Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass);

        // �������������ٹ�����Դ
        ~Pipeline();

        // ������ɫ���飨����/Ƭ�εȣ�
        void setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup);

        // �����ӿڲ����������ö���ӿڣ�
        void setViewports(const std::vector<VkViewport>& viewports) { mViewports = viewports; }

        // ���òü����򣨿����ö���ü�����
        void setScissors(const std::vector<VkRect2D>& scissors) { mScissors = scissors; }

        // ��ӻ�ϸ���״̬��������ɫ��ϣ�
        void pushBlendAttachment(const VkPipelineColorBlendAttachmentState& blendAttachment)
        {
            mBlendAttachmentStates.push_back(blendAttachment);
        }

        // ����ͼ�ι��ߣ�����������ú���ã�
        void build();

    public:
        // === ����״̬���� ===
        VkPipelineVertexInputStateCreateInfo mVertexInputState{};                   // ��������״̬
        VkPipelineInputAssemblyStateCreateInfo mAssemblyState{};                    // ͼԪװ��״̬
        VkPipelineViewportStateCreateInfo mViewportState{};                         // �ӿ�״̬
        VkPipelineRasterizationStateCreateInfo mRasterState{};                      // ��դ��״̬
        VkPipelineMultisampleStateCreateInfo mSampleState{};                        // ���ز���״̬
        std::vector<VkPipelineColorBlendAttachmentState> mBlendAttachmentStates{};  // ��ɫ��ϸ���״̬
        VkPipelineColorBlendStateCreateInfo mBlendState{};                          // ȫ�ֻ��״̬
        VkPipelineDepthStencilStateCreateInfo mDepthStencilState{};                 // ���/ģ�����״̬
        VkPipelineLayoutCreateInfo mLayoutState{};                                  // ���߲���״̬

    public:
        // === ���ʷ��� ===
        [[nodiscard]] auto getPipeline() const { return mPipeline; }                // ��ȡ���߶���
        [[nodiscard]] auto getLayout() const { return mLayout; }                    // ��ȡ���߲���

    private:
        VkPipeline mPipeline{ VK_NULL_HANDLE };      // Vulkan ���߶�����
        VkPipelineLayout mLayout{ VK_NULL_HANDLE };  // Vulkan ���߲��־��
        Device::Ptr mDevice{ nullptr };              // �߼��豸����
        RenderPass::Ptr mRenderPass{ nullptr };      // ��Ⱦͨ������

        std::vector<Shader::Ptr> mShaders{};         // ��ɫ�����󼯺�
        std::vector<VkViewport> mViewports{};        // �ӿ������б�
        std::vector<VkRect2D> mScissors{};           // �ü������б�
    };
}