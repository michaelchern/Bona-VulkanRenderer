#include "application.h"

namespace LearnVulkan
{
    void Application::run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanUp();
    }

    void Application::initWindow()
    {
        mWindow = Wrapper::Window::create(mWidth, mHeight);
    }

    void Application::initVulkan()
    {
        mInstance = Wrapper::Instance::create(true);

        mSurface = Wrapper::WindowSurface::create(mInstance, mWindow);

        mDevice = Wrapper::Device::create(mInstance, mSurface);

        mCommandPool = Wrapper::CommandPool::create(mDevice);

        mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
        mWidth  = mSwapChain->getExtent().width;
        mHeight = mSwapChain->getExtent().height;

        mRenderPass = Wrapper::RenderPass::create(mDevice);
        createRenderPass();

        mSwapChain->createFrameBuffers(mRenderPass);

        mUniformManager = UniformManager::create();
        mUniformManager->init(mDevice, mCommandPool, mSwapChain->getImageCount());

        mModel = Model::create(mDevice);

        mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
        createPipeline();

        mCommandBuffers.resize(mSwapChain->getImageCount());
        createCommandBuffers();

        createSyncObjects();
    }

    void Application::createPipeline()
    {
        VkViewport viewport = {};
        viewport.x        = 0.0f;
        viewport.y        = (float)mHeight;
        viewport.width    = (float)mWidth;
        viewport.height   = -(float)mHeight;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset   = { 0, 0 };
        scissor.extent   = { mWidth, mHeight };

        mPipeline->setViewports({ viewport });
        mPipeline->setScissors({ scissor });

        std::vector<Wrapper::Shader::Ptr> shaderGroup{};

        auto shaderVertex = Wrapper::Shader::create(mDevice, "shaders/vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
        shaderGroup.push_back(shaderVertex);

        auto shaderFragment = Wrapper::Shader::create(mDevice, "shaders/fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
        shaderGroup.push_back(shaderFragment);

        mPipeline->setShaderGroup(shaderGroup);

        auto bindingDescription = mModel->getBindingDescription();
        auto attributeDescriptions = mModel->getAttributeDescriptions();

        mPipeline->mVertexInputState.vertexBindingDescriptionCount   = 1;
        mPipeline->mVertexInputState.pVertexBindingDescriptions      = &bindingDescription;
        mPipeline->mVertexInputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
        mPipeline->mVertexInputState.pVertexAttributeDescriptions    = attributeDescriptions.data();

        mPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        mPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        mPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

        mPipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        mPipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;           
        mPipeline->mRasterState.lineWidth = 1.0f;                             
        mPipeline->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT;             
        mPipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;  

        mPipeline->mRasterState.depthBiasEnable         = VK_FALSE;
        mPipeline->mRasterState.depthBiasConstantFactor = 0.0f;
        mPipeline->mRasterState.depthBiasClamp          = 0.0f;
        mPipeline->mRasterState.depthBiasSlopeFactor    = 0.0f;

        mPipeline->mSampleState.sampleShadingEnable   = VK_FALSE;
        mPipeline->mSampleState.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        mPipeline->mSampleState.minSampleShading      = 1.0f;
        mPipeline->mSampleState.pSampleMask           = nullptr;
        mPipeline->mSampleState.alphaToCoverageEnable = VK_FALSE;
        mPipeline->mSampleState.alphaToOneEnable      = VK_FALSE;

        mPipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
        mPipeline->mDepthStencilState.depthWriteEnable = VK_TRUE;
        mPipeline->mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                         VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT |
                                         VK_COLOR_COMPONENT_A_BIT;

        blendAttachment.blendEnable         = VK_FALSE;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;

        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

        mPipeline->pushBlendAttachment(blendAttachment);

        mPipeline->mBlendState.logicOpEnable = VK_FALSE;
        mPipeline->mBlendState.logicOp = VK_LOGIC_OP_COPY;

        mPipeline->mBlendState.blendConstants[0] = 0.0f;
        mPipeline->mBlendState.blendConstants[1] = 0.0f;
        mPipeline->mBlendState.blendConstants[2] = 0.0f;
        mPipeline->mBlendState.blendConstants[3] = 0.0f;

        mPipeline->mLayoutState.setLayoutCount = 1;
        auto layout = mUniformManager->getDescriptorLayout()->getLayout();
        mPipeline->mLayoutState.pSetLayouts = &layout;

        mPipeline->mLayoutState.pushConstantRangeCount = 0;
        mPipeline->mLayoutState.pPushConstantRanges = nullptr;

        mPipeline->build();
    }

    void Application::createRenderPass()
    {
        VkAttachmentDescription attachmentDes{};
        attachmentDes.format         = mSwapChain->getFormat();           
        attachmentDes.samples        = VK_SAMPLE_COUNT_1_BIT;             
        attachmentDes.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;       
        attachmentDes.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;      
        attachmentDes.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   
        attachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDes.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDes.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        mRenderPass->addAttachment(attachmentDes);

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format         = Wrapper::Image::findDepthFormat(mDevice);
        depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        mRenderPass->addAttachment(depthAttachment);

        VkAttachmentReference attachmentRef{};
        attachmentRef.attachment = 0;
        attachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthattachmentRef{};
        depthattachmentRef.attachment = 1;
        depthattachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        Wrapper::SubPass subPass{};
        subPass.addColorAttachmentReference(attachmentRef);
        subPass.setDepthStencilAttachmentReference(depthattachmentRef);
        subPass.buildSubPassDescription();

        mRenderPass->addSubPass(subPass);

   
        VkSubpassDependency dependency{};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;                            // 外部依赖
        dependency.dstSubpass    = 0;                                              // 依赖我们的子通道
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;  // 输出阶段
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;  // 相同阶段
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |           // 读写访问权限
                                   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        mRenderPass->addDependency(dependency);                                    // 添加依赖

        mRenderPass->buildRenderPass();  // 构建渲染通道
    }

    /**
     * @brief 创建命令缓冲区（记录渲染命令）
     */
    void Application::createCommandBuffers()
    {
        // 为每个交换链图像创建命令缓冲区
        for (int i = 0; i < mSwapChain->getImageCount(); ++i)
        {
            // 创建命令缓冲区
            mCommandBuffers[i] = Wrapper::CommandBuffer::create(mDevice, mCommandPool);

            // 开始记录命令
            mCommandBuffers[i]->begin();

            // 配置渲染通道开始信息
            VkRenderPassBeginInfo renderBeginInfo{};
            renderBeginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderBeginInfo.renderPass        = mRenderPass->getRenderPass();
            renderBeginInfo.framebuffer       = mSwapChain->getFrameBuffer(i);
            renderBeginInfo.renderArea.offset = { 0, 0 };
            renderBeginInfo.renderArea.extent = mSwapChain->getExtent();

            std::vector<VkClearValue> clearColors;
            VkClearValue clearColor;
            clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
            clearColors.push_back(clearColor);

            VkClearValue depthClearColor;
            depthClearColor.depthStencil = { 1.0f, 0 };
            clearColors.push_back(depthClearColor);

            renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
            renderBeginInfo.pClearValues    = clearColors.data();

            mCommandBuffers[i]->beginRenderPass(renderBeginInfo);

            mCommandBuffers[i]->bindGraphicPipeline(mPipeline->getPipeline());

            mCommandBuffers[i]->bindDescriptorSet(mPipeline->getLayout(), mUniformManager->getDescriptorSet(mCurrentFrame));

            //mCommandBuffers[i]->bindVertexBuffer({ mModel->getVertexBuffer()->getBuffer() });

            mCommandBuffers[i]->bindVertexBuffer(mModel->getVertexBuffers());

            mCommandBuffers[i]->bindIndexBuffer(mModel->getIndexBuffer()->getBuffer());

            mCommandBuffers[i]->drawIndex(mModel->getIndexCount());

            mCommandBuffers[i]->endRenderPass();

            mCommandBuffers[i]->end();
        }
    }

    void Application::createSyncObjects()
    {
        for (int i = 0; i < mSwapChain->getImageCount(); ++i)
        {
            auto imageSemaphore = Wrapper::Semaphore::create(mDevice);
            mImageAvailableSemaphores.push_back(imageSemaphore);

            auto renderSemaphore = Wrapper::Semaphore::create(mDevice);
            mRenderFinishedSemaphores.push_back(renderSemaphore);

            auto fence = Wrapper::Fence::create(mDevice);
            mFences.push_back(fence);
        }
    }

    void Application::recreateSwapChain()
    {
        // 获取窗口实际尺寸
        int width = 0, height = 0;
        glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);

        // 等待窗口恢复有效大小
        while (width == 0 || height == 0)
        {
            glfwWaitEvents();  // 暂停直到窗口事件
            glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
        }

        // 等待设备空闲（避免资源使用中）
        vkDeviceWaitIdle(mDevice->getDevice());

        // 清理旧交换链相关资源
        cleanupSwapChain();

        // 重建交换链（新尺寸）
        mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
        mWidth = mSwapChain->getExtent().width;
        mHeight = mSwapChain->getExtent().height;

        // 重建渲染通道
        mRenderPass = Wrapper::RenderPass::create(mDevice);
        createRenderPass();

        // 重建帧缓冲区
        mSwapChain->createFrameBuffers(mRenderPass);

        // 重建图形管线（适配新尺寸）
        mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
        createPipeline();

        // 重建命令缓冲区
        mCommandBuffers.resize(mSwapChain->getImageCount());
        createCommandBuffers();

        // 重建同步对象
        createSyncObjects();
    }

    /**
     * @brief 清理交换链相关资源
     */
    void Application::cleanupSwapChain()
    {
        mSwapChain.reset();
        mCommandBuffers.clear();
        mPipeline.reset();
        mRenderPass.reset();
        mImageAvailableSemaphores.clear();
        mRenderFinishedSemaphores.clear();
        mFences.clear();
    }

    /**
     * @brief 主渲染循环
     */
    void Application::mainLoop()
    {
        // 窗口未关闭时循环
        while (!mWindow->shouldClose())
        {
            mWindow->pollEvents();  // 处理窗口事件

            mModel->update();       // 更新模型状态

            // 更新Uniform数据（视图/投影矩阵 + 模型矩阵）
            mUniformManager->update(mVPMatrices, mModel->getUniform(), mCurrentFrame);

            render();               // 渲染一帧
        }

        // 等待所有操作完成后再退出
        vkDeviceWaitIdle(mDevice->getDevice());
    }

    /**
     * @brief 渲染单帧
     */
    void Application::render()
    {
        // 等待当前要提交的CommandBuffer执行完毕
        // 等待当前帧的栅栏（确保该帧的命令缓冲区已完成）
        mFences[mCurrentFrame]->block();

        // 获取交换链当中的下一帧
        uint32_t imageIndex{ 0 };
        VkResult result = vkAcquireNextImageKHR(mDevice->getDevice(),
                                                mSwapChain->getSwapChain(),
                                                UINT64_MAX,  // 无超时限制
                                                mImageAvailableSemaphores[mCurrentFrame]->getSemaphore(),
                                                VK_NULL_HANDLE,
                                                &imageIndex);

        // 处理交换链失效情况（窗口大小变化）
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            mWindow->mWindowResized = false;
        }//VK_SUBOPTIMAL_KHR得到了一张认为可用的图像，但是表面格式不一定匹配
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Error: failed to acquire next image!");
        }

        // 构建提交信息
        // 配置命令缓冲区提交信息
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // 同步信息，渲染对于显示图像的依赖，显示完毕后，才能输出颜色
        VkSemaphore waitSemaphores[]      = { mImageAvailableSemaphores[mCurrentFrame]->getSemaphore() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount     = 1;
        submitInfo.pWaitSemaphores        = waitSemaphores;
        submitInfo.pWaitDstStageMask      = waitStages;

        // 指定提交哪些命令
        // 指定要提交的命令缓冲区
        auto commandBuffer            = mCommandBuffers[imageIndex]->getCommandBuffer();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        // 设置完成信号量（渲染完成）
        VkSemaphore signalSemaphores[]  = { mRenderFinishedSemaphores[mCurrentFrame]->getSemaphore() };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        // 重置栅栏（准备新一轮提交）
        mFences[mCurrentFrame]->resetFence();

        // 提交命令缓冲区到图形队列
        if (vkQueueSubmit(mDevice->getGraphicQueue(), 1, &submitInfo, mFences[mCurrentFrame]->getFence()) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to submit renderCommand!");
        }

        // 等待渲染完成信号量，准备呈现图像
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;  // 等待渲染完成

        VkSwapchainKHR swapChains[] = { mSwapChain->getSwapChain() };
        presentInfo.swapchainCount  = 1;
        presentInfo.pSwapchains     = swapChains;
        presentInfo.pImageIndices   = &imageIndex;

        // 提交呈现请求
        result = vkQueuePresentKHR(mDevice->getPresentQueue(), &presentInfo);

        // 由于驱动程序不一定精准，所以我们还需要用自己的标志位判断
        // 检查呈现结果（处理大小变化）
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mWindow->mWindowResized)
        {
            recreateSwapChain();
            mWindow->mWindowResized = false;
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to present!");
        }

        // 更新当前帧索引（循环使用交换链图像）
        mCurrentFrame = (mCurrentFrame + 1) % mSwapChain->getImageCount();
    }

    void Application::cleanUp()
    {
        mPipeline.reset();
        mRenderPass.reset();  // 销毁渲染通道
        mSwapChain.reset();   // 销毁交换链
        mDevice.reset();      // 销毁设备
        mSurface.reset();     // 销毁窗口表面
        mInstance.reset();    // 销毁Vulkan实例
        mWindow.reset();      // 销毁窗口
    }
}
