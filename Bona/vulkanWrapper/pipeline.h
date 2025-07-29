#pragma once

#include "base.h"
#include "device.h"
#include "shader.h"
#include "renderPass.h"

namespace LearnVulkan::Wrapper
{
    class Pipeline
    {
    public:
        using Ptr = std::shared_ptr<Pipeline>;

        static Ptr create(const Device::Ptr& device, const RenderPass::Ptr& renderPass)
        {
            return std::make_shared<Pipeline>(device, renderPass);
        }

        Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass);

        ~Pipeline();

        void setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup);

        void setViewports(const std::vector<VkViewport>& viewports) { mViewports = viewports; }

        void setScissors(const std::vector<VkRect2D>& scissors) { mScissors = scissors; }

        void pushBlendAttachment(const VkPipelineColorBlendAttachmentState& blendAttachment)
        {
            mBlendAttachmentStates.push_back(blendAttachment);
        }

        void build();

    public:
        VkPipelineVertexInputStateCreateInfo             mVertexInputState{};
        VkPipelineInputAssemblyStateCreateInfo           mAssemblyState{};
        VkPipelineViewportStateCreateInfo                mViewportState{};
        VkPipelineRasterizationStateCreateInfo           mRasterState{};
        VkPipelineMultisampleStateCreateInfo             mSampleState{};
        std::vector<VkPipelineColorBlendAttachmentState> mBlendAttachmentStates{};
        VkPipelineColorBlendStateCreateInfo              mBlendState{};
        VkPipelineDepthStencilStateCreateInfo            mDepthStencilState{};
        VkPipelineLayoutCreateInfo                       mLayoutState{};

    public:
        [[nodiscard]] auto getPipeline() const { return mPipeline; }
        [[nodiscard]] auto getLayout()   const { return mLayout; }

    private:
        VkPipeline       mPipeline{ VK_NULL_HANDLE };
        VkPipelineLayout mLayout{ VK_NULL_HANDLE };
        Device::Ptr      mDevice{ nullptr };
        RenderPass::Ptr  mRenderPass{ nullptr };

        std::vector<Shader::Ptr> mShaders{};
        std::vector<VkViewport>  mViewports{};
        std::vector<VkRect2D>    mScissors{};
    };
}