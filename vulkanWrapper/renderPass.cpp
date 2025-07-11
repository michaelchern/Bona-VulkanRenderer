
#include "renderPass.h"

namespace LearnVulkan::Wrapper
{
    SubPass::SubPass()
    {
    }

    SubPass::~SubPass()
    {
    }

    void SubPass::addColorAttachmentReference(const VkAttachmentReference& ref)
    {
        mColorAttachmentReferences.push_back(ref);
    }

    void SubPass::addInputAttachmentReference(const VkAttachmentReference& ref)
    {
        mInputAttachmentReferences.push_back(ref);
    }

    void SubPass::setDepthStencilAttachmentReference(const VkAttachmentReference& ref)
    {
        mDepthStencilAttachmentReference = ref;
    }

    void SubPass::buildSubPassDescription()
    {
        if (mColorAttachmentReferences.empty())
        {
            throw std::runtime_error("Error: color attachment group is empty!");
        }

        mSubPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        mSubPassDescription.colorAttachmentCount = static_cast<uint32_t>(mColorAttachmentReferences.size());
        mSubPassDescription.pColorAttachments = mColorAttachmentReferences.data();

        mSubPassDescription.inputAttachmentCount = static_cast<uint32_t>(mInputAttachmentReferences.size());
        mSubPassDescription.pInputAttachments = mInputAttachmentReferences.data();

        mSubPassDescription.pDepthStencilAttachment = mDepthStencilAttachmentReference.layout == VK_IMAGE_LAYOUT_UNDEFINED ? nullptr : &mDepthStencilAttachmentReference;
    }

    /************************ RenderPass 实现 ************************/

    /**
     * @brief RenderPass 构造函数
     * @param device 关联的逻辑设备
     */
    RenderPass::RenderPass(const Device::Ptr& device)
    {
        mDevice = device;
    }

    /**
     * @brief RenderPass 析构函数
     *
     * 销毁Vulkan渲染通道资源
     */
    RenderPass::~RenderPass()
    {
        if (mRenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(mDevice->getDevice(), mRenderPass, nullptr);
        }
    }

    void RenderPass::addSubPass(const SubPass& subpass) { mSubPasses.push_back(subpass); }

    void RenderPass::addDependency(const VkSubpassDependency& dependency) { mDependencies.push_back(dependency); }

    void RenderPass::addAttachment(const VkAttachmentDescription& attachmentDes) { mAttachmentDescriptions.push_back(attachmentDes); }

    void RenderPass::buildRenderPass()
    {
		// 安全校验：必须至少有一个子通道、一个附件描述和一个依赖关系
        if (mSubPasses.empty() || mAttachmentDescriptions.empty() || mDependencies.empty())
        {
            throw std::runtime_error("Error: not enough elements to build renderPass!");
        }

        // 解包子通道描述（转换为VkSubpassDescription数组）
        std::vector<VkSubpassDescription> subPasses{};
        for (int i = 0; i < mSubPasses.size(); ++i)
        {
            subPasses.push_back(mSubPasses[i].getSubPassDescription());
        }

		// 创建渲染通道描述信息
        VkRenderPassCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

		// 设置渲染通道的附件描述
        createInfo.attachmentCount = static_cast<uint32_t>(mAttachmentDescriptions.size());
        createInfo.pAttachments = mAttachmentDescriptions.data();

		// 设置子通道依赖关系
        createInfo.dependencyCount = static_cast<uint32_t>(mDependencies.size());
        createInfo.pDependencies = mDependencies.data();

		// 设置子通道描述
        createInfo.subpassCount = static_cast<uint32_t>(subPasses.size());
        createInfo.pSubpasses = subPasses.data();

		// 创建渲染通道
        if (vkCreateRenderPass(mDevice->getDevice(), &createInfo, nullptr, &mRenderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create renderPass!");
        }
    }
}