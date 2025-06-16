
#include "application.h"

namespace LearnVulkan
{
	// 主运行函数：初始化 -> 主循环 -> 清理
	void Application::run()
	{
		initWindow();  // 初始化GLFW窗口
		initVulkan();  // 初始化Vulkan组件
		mainLoop();    // 进入渲染主循环
		cleanUp();     // 清理资源
	}

	// 初始化GLFW窗口
	void Application::initWindow()
	{
		mWindow = Wrapper::Window::create(mWidth, mHeight);
	}

	// 初始化Vulkan组件
	void Application::initVulkan()
	{
		// 1. 创建Vulkan实例
		mInstance = Wrapper::Instance::create(true);

		// 2. 创建窗口表面
		mSurface = Wrapper::WindowSurface::create(mInstance, mWindow);

		// 3. 创建设备（物理设备和逻辑设备）
		mDevice = Wrapper::Device::create(mInstance, mSurface);

		// 4. 创建交换链（管理帧缓冲）
		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface);
		mWidth = mSwapChain->getExtent().width;    // 更新实际宽度
		mHeight = mSwapChain->getExtent().height;  // 更新实际高度

		// 5. 创建渲染通道
		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();  // 配置渲染通道

		// 6. 为交换链创建帧缓冲区
		mSwapChain->createFrameBuffers(mRenderPass);


		// 7. 创建命令池（管理命令缓冲区）
		mCommandPool = Wrapper::CommandPool::create(mDevice);

		// 8. 初始化Uniform管理器（管理着色器常量）
		mUniformManager = UniformManager::create();
		mUniformManager->init(mDevice, mCommandPool, mSwapChain->getImageCount());

		// 9. 创建模型（加载几何数据）
		mModel = Model::create(mDevice);

		// 10. 创建图形管线
		mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createPipeline();  // 配置管线

		// 11. 创建命令缓冲区（每个交换链图像一个）
		mCommandBuffers.resize(mSwapChain->getImageCount());
		createCommandBuffers();  // 记录渲染命令

		// 12. 创建同步对象（帧同步）
		createSyncObjects();
	}

	// 配置并创建图形管线
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

		mPipeline->setViewports({ viewport });
		mPipeline->setScissors({ scissor });

		// 设置着色器
		std::vector<Wrapper::Shader::Ptr> shaderGroup{};

		auto shaderVertex = Wrapper::Shader::create(mDevice, "shaders/vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		shaderGroup.push_back(shaderVertex);

		auto shaderFragment = Wrapper::Shader::create(mDevice, "shaders/fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		shaderGroup.push_back(shaderFragment);

		mPipeline->setShaderGroup(shaderGroup);

		//顶点的排布模式
		auto vertexBindingDes = mModel->getVertexInputBindingDescriptions();
		auto attributeDes = mModel->getAttributeDescriptions();

		mPipeline->mVertexInputState.vertexBindingDescriptionCount = vertexBindingDes.size();
		mPipeline->mVertexInputState.pVertexBindingDescriptions = vertexBindingDes.data();
		mPipeline->mVertexInputState.vertexAttributeDescriptionCount = attributeDes.size();
		mPipeline->mVertexInputState.pVertexAttributeDescriptions = attributeDes.data();

		//图元装配
		mPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		//光栅化设置
		mPipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mPipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;//其他模式需要开启gpu特性
		mPipeline->mRasterState.lineWidth = 1.0f;//大于1需要开启gpu特性
		mPipeline->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT;
		mPipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mPipeline->mRasterState.depthBiasEnable = VK_FALSE;
		mPipeline->mRasterState.depthBiasConstantFactor = 0.0f;
		mPipeline->mRasterState.depthBiasClamp = 0.0f;
		mPipeline->mRasterState.depthBiasSlopeFactor = 0.0f;

		//TODO:多重采样
		mPipeline->mSampleState.sampleShadingEnable = VK_FALSE;
		mPipeline->mSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		mPipeline->mSampleState.minSampleShading = 1.0f;
		mPipeline->mSampleState.pSampleMask = nullptr;
		mPipeline->mSampleState.alphaToCoverageEnable = VK_FALSE;
		mPipeline->mSampleState.alphaToOneEnable = VK_FALSE;

		//TODO:深度与模板测试

		//颜色混合

		//这个是颜色混合掩码，得到的混合结果，按照通道与掩码进行AND操作，输出
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

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

		//uniform的传递
		mPipeline->mLayoutState.setLayoutCount = 1;

		auto layout = mUniformManager->getDescriptorLayout()->getLayout();
		mPipeline->mLayoutState.pSetLayouts = &layout;
		mPipeline->mLayoutState.pushConstantRangeCount = 0;
		mPipeline->mLayoutState.pPushConstantRanges = nullptr;

		mPipeline->build();
	}

	void Application::createRenderPass() {
		//输入画布的描述
		VkAttachmentDescription attachmentDes{};
		attachmentDes.format = mSwapChain->getFormat();
		attachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDes.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		mRenderPass->addAttachment(attachmentDes);

		//对于画布的索引设置以及格式要求
		VkAttachmentReference attachmentRef{};
		attachmentRef.attachment = 0;
		attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//创建子流程
		Wrapper::SubPass subPass{};
		subPass.addColorAttachmentReference(attachmentRef);
		subPass.buildSubPassDescription();

		mRenderPass->addSubPass(subPass);

		//子流程之间的依赖关系
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		mRenderPass->addDependency(dependency);

		mRenderPass->buildRenderPass();
	}

	void Application::createCommandBuffers() {
		for (int i = 0; i < mSwapChain->getImageCount(); ++i) {
			mCommandBuffers[i] = Wrapper::CommandBuffer::create(mDevice, mCommandPool);

			mCommandBuffers[i]->begin();

			VkRenderPassBeginInfo renderBeginInfo{};
			renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderBeginInfo.renderPass = mRenderPass->getRenderPass();
			renderBeginInfo.framebuffer = mSwapChain->getFrameBuffer(i);
			renderBeginInfo.renderArea.offset = { 0, 0 };
			renderBeginInfo.renderArea.extent = mSwapChain->getExtent();

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderBeginInfo.clearValueCount = 1;
			renderBeginInfo.pClearValues = &clearColor;


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

	void Application::createSyncObjects() {
		for (int i = 0; i < mSwapChain->getImageCount(); ++i) {
			auto imageSemaphore = Wrapper::Semaphore::create(mDevice);
			mImageAvailableSemaphores.push_back(imageSemaphore);

			auto renderSemaphore = Wrapper::Semaphore::create(mDevice);
			mRenderFinishedSemaphores.push_back(renderSemaphore);

			auto fence = Wrapper::Fence::create(mDevice);
			mFences.push_back(fence);
		}
	}

	void Application::recreateSwapChain() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
		while (width == 0 || height == 0) {
			glfwWaitEvents();
			glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
		}

		vkDeviceWaitIdle(mDevice->getDevice());

		cleanupSwapChain();

		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface);
		mWidth = mSwapChain->getExtent().width;
		mHeight = mSwapChain->getExtent().height;

		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();

		mSwapChain->createFrameBuffers(mRenderPass);

		mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createPipeline();

		mCommandBuffers.resize(mSwapChain->getImageCount());

		createCommandBuffers();

		createSyncObjects();
	}

	void Application::cleanupSwapChain() {
		mSwapChain.reset();
		mCommandBuffers.clear();
		mPipeline.reset();
		mRenderPass.reset();
		mImageAvailableSemaphores.clear();
		mRenderFinishedSemaphores.clear();
		mFences.clear();
	}


	void Application::mainLoop() {
		while (!mWindow->shouldClose()) {
			mWindow->pollEvents();

			mModel->update();

			mUniformManager->update(mVPMatrices, mModel->getUniform(), mCurrentFrame);

			render();
		}

		vkDeviceWaitIdle(mDevice->getDevice());
	}

	void Application::render() {
		//等待当前要提交的CommandBuffer执行完毕
		mFences[mCurrentFrame]->block();

		//获取交换链当中的下一帧
		uint32_t imageIndex{ 0 };
		VkResult result = vkAcquireNextImageKHR(
			mDevice->getDevice(),
			mSwapChain->getSwapChain(),
			UINT64_MAX,
			mImageAvailableSemaphores[mCurrentFrame]->getSemaphore(),
			VK_NULL_HANDLE,
			&imageIndex);

		//窗体发生了尺寸变化
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			mWindow->mWindowResized = false;
		}//VK_SUBOPTIMAL_KHR得到了一张认为可用的图像，但是表面格式不一定匹配
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Error: failed to acquire next image");
		}

		//构建提交信息
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;


		//同步信息，渲染对于显示图像的依赖，显示完毕后，才能输出颜色
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame]->getSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//指定提交哪些命令
		auto commandBuffer = mCommandBuffers[imageIndex]->getCommandBuffer();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame]->getSemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		mFences[mCurrentFrame]->resetFence();
		if (vkQueueSubmit(mDevice->getGraphicQueue(), 1, &submitInfo, mFences[mCurrentFrame]->getFence()) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to submit renderCommand");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChain->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(mDevice->getPresentQueue(), &presentInfo);

		//由于驱动程序不一定精准，所以我们还需要用自己的标志位判断
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mWindow->mWindowResized) {
			recreateSwapChain();
			mWindow->mWindowResized = false;
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to present");
		}

		mCurrentFrame = (mCurrentFrame + 1) % mSwapChain->getImageCount();
	}

	void Application::cleanUp()
	{
		mPipeline.reset();
		mRenderPass.reset();
		mSwapChain.reset();
		mDevice.reset();
		mSurface.reset();
		mInstance.reset();
		mWindow.reset();
	}
}