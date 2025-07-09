
#include "application.h"

namespace LearnVulkan
{
    /**
     * @brief 应用程序主循环
     *
     * 执行顺序：
     *  1. 初始化窗口
     *  2. 初始化Vulkan
     *  3. 进入渲染主循环
     *  4. 清理资源
     */
    void Application::run()
    {
        initWindow();  // 创建GLFW窗口
        initVulkan();  // 初始化所有Vulkan资源
        mainLoop();    // 主渲染循环
        cleanUp();     // 清理所有资源
    }

    /**
     * @brief 初始化Vulkan组件
     *
     * 步骤说明：
     *  1. 创建Vulkan实例
     *  2. 创建窗口表面
     *  3. 创建设备（物理设备+逻辑设备）
     *  4. 创建交换链
     *  5. 创建渲染通道
     *  6. 创建帧缓冲区
     *  7. 创建命令池
     *  8. 初始化Uniform管理器
     *  9. 加载模型
     * 10. 创建图形管线
     * 11. 创建命令缓冲区
     * 12. 创建同步对象
     */
    void Application::initWindow()
    {
        mWindow = Wrapper::Window::create(mWidth, mHeight);  // 创建指定大小的窗口
    }

    // 初始化Vulkan组件
    void Application::initVulkan()
    {
        // 1. 创建Vulkan实例（启用验证层）
        mInstance = Wrapper::Instance::create(true);

        // 2. 创建窗口表面（连接GLFW窗口和Vulkan）
        mSurface = Wrapper::WindowSurface::create(mInstance, mWindow);

        // 3. 创建设备（选择物理设备并创建逻辑设备）
        mDevice = Wrapper::Device::create(mInstance, mSurface);

        // 4. 创建交换链（管理图像缓冲区）
        mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface);
        // 更新窗口实际尺寸（交换链可能调整了大小）
        mWidth = mSwapChain->getExtent().width;
        mHeight = mSwapChain->getExtent().height;

        // 5. 创建渲染通道
        mRenderPass = Wrapper::RenderPass::create(mDevice);
        createRenderPass();  // 自定义函数配置渲染通道

        // 6. 为交换链创建帧缓冲区
        mSwapChain->createFrameBuffers(mRenderPass);

        // 7. 创建命令池（用于分配命令缓冲区）
        mCommandPool = Wrapper::CommandPool::create(mDevice);

        // 8. 初始化Uniform管理器（管理着色器常量）
        mUniformManager = UniformManager::create();
        mUniformManager->init(mDevice, mCommandPool, mSwapChain->getImageCount());

        // 9. 创建模型（加载顶点/索引数据）
        mModel = Model::create(mDevice);

        // 10. 创建图形管线
        mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
        createPipeline();  // 自定义函数配置管线

        // 11. 创建命令缓冲区（每个交换链图像一个）
        mCommandBuffers.resize(mSwapChain->getImageCount());
        createCommandBuffers();  // 自定义函数记录渲染命令

        // 12. 创建同步对象（帧同步）
        createSyncObjects();
    }

    /**
     * @brief 配置并创建图形管线
     *
     * 配置内容：
     *  1. 视口设置（翻转Y轴）
     *  2. 裁剪区域
     *  3. 着色器
     *  4. 顶点输入格式
     *  5. 图元装配方式
     *  6. 光栅化设置
     *  7. 多重采样
     *  8. 颜色混合
     *  9. Uniform布局
     */
    void Application::createPipeline()
    {
        // 设置视口（翻转Y轴使坐标原点在左上角）
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = (float)mHeight;        // 从底部开始
        viewport.width = (float)mWidth;
        viewport.height = -(float)mHeight;  // 负值表示翻转Y轴
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        // 设置裁剪区域（整个窗口）
        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = { mWidth, mHeight };

        // 应用视口和裁剪配置
        mPipeline->setViewports({ viewport });
        mPipeline->setScissors({ scissor });

        // 设置着色器
        std::vector<Wrapper::Shader::Ptr> shaderGroup{};

        // 创建顶点着色器
        auto shaderVertex = Wrapper::Shader::create(mDevice, "shaders/vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
        shaderGroup.push_back(shaderVertex);

        // 创建片段着色器
        auto shaderFragment = Wrapper::Shader::create(mDevice, "shaders/fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
        shaderGroup.push_back(shaderFragment);

        mPipeline->setShaderGroup(shaderGroup);

        // 配置顶点输入格式
        auto vertexBindingDes = mModel->getVertexInputBindingDescriptions();
        auto attributeDes = mModel->getAttributeDescriptions();

        mPipeline->mVertexInputState.vertexBindingDescriptionCount   = vertexBindingDes.size();
        mPipeline->mVertexInputState.pVertexBindingDescriptions      = vertexBindingDes.data();
        mPipeline->mVertexInputState.vertexAttributeDescriptionCount = attributeDes.size();
        mPipeline->mVertexInputState.pVertexAttributeDescriptions    = attributeDes.data();

        // 配置图元装配方式
        mPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        mPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // 三角形列表
        mPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;               // 禁用图元重启

        // 配置光栅化
        mPipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        mPipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;           // 实心填充
        mPipeline->mRasterState.lineWidth = 1.0f;                             // 线宽1像素
        mPipeline->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT;             // 背面剔除
        mPipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;  // 逆时针为正面

        // 禁用深度偏移
        mPipeline->mRasterState.depthBiasEnable         = VK_FALSE;
        mPipeline->mRasterState.depthBiasConstantFactor = 0.0f;
        mPipeline->mRasterState.depthBiasClamp          = 0.0f;
        mPipeline->mRasterState.depthBiasSlopeFactor    = 0.0f;

        // 配置多重采样（当前禁用）
        mPipeline->mSampleState.sampleShadingEnable   = VK_FALSE;
        mPipeline->mSampleState.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        mPipeline->mSampleState.minSampleShading      = 1.0f;
        mPipeline->mSampleState.pSampleMask           = nullptr;
        mPipeline->mSampleState.alphaToCoverageEnable = VK_FALSE;
        mPipeline->mSampleState.alphaToOneEnable      = VK_FALSE;
        // 其他采样参数保持默认

         // 配置颜色混合状态
        VkPipelineColorBlendAttachmentState blendAttachment{};
        // 启用所有颜色通道
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                         VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT |
                                         VK_COLOR_COMPONENT_A_BIT;
        // 禁用混合
        blendAttachment.blendEnable         = VK_FALSE;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;

        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

        mPipeline->pushBlendAttachment(blendAttachment);

        //1 blend有两种计算方式，第一种如上所述，进行alpha为基础的计算，第二种进行位运算
        //2 如果开启了logicOp，那么上方设置的alpha为基础的运算，失灵
        //3 ColorWrite掩码，仍然有效，即便开启了logicOP
        //4 因为，我们可能会有多个FrameBuffer输出，所以可能需要多个BlendAttachment
        mPipeline->mBlendState.logicOpEnable = VK_FALSE;
        mPipeline->mBlendState.logicOp = VK_LOGIC_OP_COPY;

        //配合blendAttachment的factor与operation
        mPipeline->mBlendState.blendConstants[0] = 0.0f;
        mPipeline->mBlendState.blendConstants[1] = 0.0f;
        mPipeline->mBlendState.blendConstants[2] = 0.0f;
        mPipeline->mBlendState.blendConstants[3] = 0.0f;

        // 配置Uniform布局
        mPipeline->mLayoutState.setLayoutCount = 1;
        auto layout = mUniformManager->getDescriptorLayout()->getLayout();
        mPipeline->mLayoutState.pSetLayouts = &layout;
        // 无推送常量
        mPipeline->mLayoutState.pushConstantRangeCount = 0;
        mPipeline->mLayoutState.pPushConstantRanges = nullptr;

        // 构建管线
        mPipeline->build();
    }

    /**
     * @brief 创建渲染通道
     */
    void Application::createRenderPass()
    {
        // 配置颜色附件描述
        VkAttachmentDescription attachmentDes{};
        attachmentDes.format         = mSwapChain->getFormat();           // 使用交换链格式
        attachmentDes.samples        = VK_SAMPLE_COUNT_1_BIT;             // 单采样
        attachmentDes.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;       // 加载时清空
        attachmentDes.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;      // 存储渲染结果
        attachmentDes.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // 不关心模板加载
        attachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // 不关心模板存储
        attachmentDes.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;         // 初始布局
        attachmentDes.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;   // 最终布局为显示源

        mRenderPass->addAttachment(attachmentDes);

        // 配置附件引用（指向第一个附件）
        VkAttachmentReference attachmentRef{};
        attachmentRef.attachment = 0;
        attachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 创建子通道
        Wrapper::SubPass subPass{};
        subPass.addColorAttachmentReference(attachmentRef);  // 添加颜色附件引用
        subPass.buildSubPassDescription();                   // 构建子通道描述

        mRenderPass->addSubPass(subPass);                    // 添加子通道到渲染通道

        // 配置子通道依赖关系
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

            // 设置清屏颜色（黑色）
            VkClearValue clearColor         = { 0.0f, 0.0f, 0.0f, 1.0f };
            renderBeginInfo.clearValueCount = 1;
            renderBeginInfo.pClearValues    = &clearColor;

            // 开始渲染通道
            mCommandBuffers[i]->beginRenderPass(renderBeginInfo);

            // 绑定图形管线
            mCommandBuffers[i]->bindGraphicPipeline(mPipeline->getPipeline());

            // 绑定Uniform描述符集
            mCommandBuffers[i]->bindDescriptorSet(mPipeline->getLayout(), mUniformManager->getDescriptorSet(mCurrentFrame));

            //mCommandBuffers[i]->bindVertexBuffer({ mModel->getVertexBuffer()->getBuffer() });

            // 绑定顶点缓冲区
            mCommandBuffers[i]->bindVertexBuffer(mModel->getVertexBuffers());

            // 绑定索引缓冲区
            mCommandBuffers[i]->bindIndexBuffer(mModel->getIndexBuffer()->getBuffer());

            // 绘制索引模型
            mCommandBuffers[i]->drawIndex(mModel->getIndexCount());

            // 结束渲染通道
            mCommandBuffers[i]->endRenderPass();

            // 结束命令记录
            mCommandBuffers[i]->end();
        }
    }

    /**
     * @brief 创建同步对象（信号量和栅栏）
     */
    void Application::createSyncObjects()
    {
        for (int i = 0; i < mSwapChain->getImageCount(); ++i)
        {
            // 图像可用信号量（表示交换链图像已准备就绪）
            auto imageSemaphore = Wrapper::Semaphore::create(mDevice);
            mImageAvailableSemaphores.push_back(imageSemaphore);

            // 渲染完成信号量（表示渲染已完成）
            auto renderSemaphore = Wrapper::Semaphore::create(mDevice);
            mRenderFinishedSemaphores.push_back(renderSemaphore);

            // 命令缓冲区执行完成栅栏（避免命令缓冲区重写）
            auto fence = Wrapper::Fence::create(mDevice);
            mFences.push_back(fence);
        }
    }

    /**
     * @brief 重建交换链（窗口大小改变时调用）
     */
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
        mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface);
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
        mSwapChain.reset();                 // 销毁交换链
        mCommandBuffers.clear();            // 清空命令缓冲区
        mPipeline.reset();                  // 销毁管线
        mRenderPass.reset();                // 销毁渲染通道
        mImageAvailableSemaphores.clear();  // 销毁信号量
        mRenderFinishedSemaphores.clear();  
        mFences.clear();                    // 销毁栅栏
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
            throw std::runtime_error("Error: Failed to acquire next image!");
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
            throw std::runtime_error("Error:failed to submit renderCommand!");
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

    /**
     * @brief 清理所有资源
     *
     * 注意：按创建顺序逆序销毁
     */
    void Application::cleanUp()
    {
        // 销毁Vulkan对象
        mPipeline.reset();    // 销毁管线
        mRenderPass.reset();  // 销毁渲染通道
        mSwapChain.reset();   // 销毁交换链
        mDevice.reset();      // 销毁设备
        mSurface.reset();     // 销毁窗口表面
        mInstance.reset();    // 销毁Vulkan实例
        mWindow.reset();      // 销毁窗口
    }
}
