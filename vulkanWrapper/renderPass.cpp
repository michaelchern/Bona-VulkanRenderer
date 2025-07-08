
#include "renderPass.h"

namespace LearnVulkan::Wrapper
{

    /**
     * @brief SubPass 构造函数
     *
     * 初始化子通道描述结构体
     */
    SubPass::SubPass()
    {
    }

    /**
     * @brief SubPass 析构函数
     *
     * 当前不涉及资源销毁（资源由RenderPass统一管理）
     */
    SubPass::~SubPass()
    {
    }

    /**
     * @brief 添加颜色附件引用
     * @param ref 颜色附件引用结构
     *
     * 将颜色附件添加到子通道的颜色附件列表中
     */
    void SubPass::addColorAttachmentReference(const VkAttachmentReference& ref)
    {
        mColorAttachmentReferences.push_back(ref);
    }

    /**
     * @brief 添加输入附件引用
     * @param ref 输入附件引用结构
     *
     * 将输入附件添加到子通道的输入附件列表中
     */
    void SubPass::addInputAttachmentReference(const VkAttachmentReference& ref)
    {
        mInputAttachmentReferences.push_back(ref);
    }

    /**
     * @brief 设置深度/模板附件引用
     * @param ref 深度/模板附件引用结构
     *
     * 每个子通道最多只能有一个深度/模板附件
     */
    void SubPass::setDepthStencilAttachmentReference(const VkAttachmentReference& ref)
    {
        mDepthStencilAttachmentReference = ref;
    }

    /**
     * @brief 构建子通道描述
     *
     * 配置子通道的完整描述信息
     * 必须在添加到RenderPass前调用
     */
    void SubPass::buildSubPassDescription()
    {
        // 安全校验：颜色附件不能为空
        if (mColorAttachmentReferences.empty())
        {
            throw std::runtime_error("Error: color attachment group is empty!");
        }

        // 绑定类型为图形管线（默认已在构造函数设置）
        mSubPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        // 设置颜色附件
        mSubPassDescription.colorAttachmentCount = static_cast<uint32_t>(mColorAttachmentReferences.size());
        mSubPassDescription.pColorAttachments = mColorAttachmentReferences.data();

        // 设置输入附件
        mSubPassDescription.inputAttachmentCount = static_cast<uint32_t>(mInputAttachmentReferences.size());
        mSubPassDescription.pInputAttachments = mInputAttachmentReferences.data();

        // 设置深度/模板附件（仅当布局有效时）
        // VK_IMAGE_LAYOUT_UNDEFINED 表示未设置
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

    /**
     * @brief 添加子通道
     * @param subpass 子通道对象
     */
    void RenderPass::addSubPass(const SubPass& subpass) { mSubPasses.push_back(subpass); }

    /**
     * @brief 添加子通道依赖
     * @param dependency 依赖关系描述
     */
    void RenderPass::addDependency(const VkSubpassDependency& dependency) { mDependencies.push_back(dependency); }

    /**
     * @brief 添加附件描述
     * @param attachmentDes 附件描述结构
     */
    void RenderPass::addAttachment(const VkAttachmentDescription& attachmentDes) { mAttachmentDescriptions.push_back(attachmentDes); }

    /**
     * @brief 构建Vulkan渲染通道
     *
     * 整合所有组件并创建渲染通道
     */
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