#include "swapChain.h"

namespace LearnVulkan::Wrapper
{
	// ���캯���������������������Դ
	SwapChain::SwapChain(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface)
	{
		mDevice = device;    // �����߼��豸����
		mWindow = window;    // ���洰������
		mSurface = surface;  // �����������

		// 1. ��ѯ������֧����Ϣ
		auto swapChainSupportInfo = querySwapChainSupportInfo();

		// 2. ѡ����ѱ����ʽ����ɫ��ʽ+ɫ�ʿռ䣩
		VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.mFormats);

		// 3. ѡ����ѳ���ģʽ����ֱͬ��/���ػ���ȣ�
		VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.mPresentModes);

		// 4. ѡ�񽻻���ͼ��ߴ磨�ֱ��ʣ�
		VkExtent2D extent = chooseExtent(swapChainSupportInfo.mCapabilities);

		// 5. ȷ��ͼ�񻺳�������˫����/�����壩
		mImageCount = swapChainSupportInfo.mCapabilities.minImageCount + 1;  // ��һ��������ٵȴ�

		// 6. ������ͼ���������ƣ�0��ʾ�����ƣ�
		if (swapChainSupportInfo.mCapabilities.maxImageCount > 0 && mImageCount > swapChainSupportInfo.mCapabilities.maxImageCount)
		{
			mImageCount = swapChainSupportInfo.mCapabilities.maxImageCount;
		}

		// 7. ��д������������Ϣ
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;  
		createInfo.surface = mSurface->getSurface();                     // �󶨴��ڱ���
		createInfo.minImageCount = mImageCount;                          // ��С��������
		createInfo.imageFormat = surfaceFormat.format;                   // ͼ���ʽ
		createInfo.imageColorSpace = surfaceFormat.colorSpace;           // ɫ�ʿռ�
		createInfo.imageExtent = extent;                                 // ͼ��ߴ�
		createInfo.imageArrayLayers = 1;                                 // ͼ����������VRӦ��Ϊ1��
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;     // ͼ����;����ɫ������

		// 8. ��������干��ģʽ
		std::vector<uint32_t> queueFamilies = { mDevice->getGraphicQueueFamily().value() , mDevice->getPresentQueueFamily().value() };

		// 8.1 ͼ�ζ��кͳ��ֶ�����ͬʱ - ��ռģʽ�����ܸ��ţ�
		if (mDevice->getGraphicQueueFamily().value() == mDevice->getPresentQueueFamily().value())
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;                     // ����Ҫָ��������
			createInfo.pQueueFamilyIndices = nullptr;
		}
		else  // 8.2 ��ͬ������ - ����ģʽ����Ҫ����
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
			createInfo.pQueueFamilyIndices = queueFamilies.data();
		}

		// 9. ������������
		createInfo.preTransform = swapChainSupportInfo.mCapabilities.currentTransform;  // ͼ��任
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                  // �����Alphaͨ��
		createInfo.presentMode = presentMode;                                           // ����ģʽ
		createInfo.clipped = VK_TRUE;                                                   // �ü����ڵ����֣�������ܣ�

		// 10. ����������
		if (vkCreateSwapchainKHR(mDevice->getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create swapChain");
		}

		// 11. ����ؼ�����
		mSwapChainFormat = surfaceFormat.format;
		mSwapChainExtent = extent;

		// 12. ��ȡ������ʵ�ʴ�����ͼ�����������ܶ�������
		vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, nullptr);
		mSwapChainImages.resize(mImageCount);
		vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, mSwapChainImages.data());

		// 13. Ϊÿ��������ͼ�񴴽�ͼ����ͼ
		mSwapChainImageViews.resize(mImageCount);
		for (int i = 0; i < mImageCount; ++i)
		{
			mSwapChainImageViews[i] = createImageView(
				mSwapChainImages[i],
				mSwapChainFormat,
				VK_IMAGE_ASPECT_COLOR_BIT,  // ��Ϊ��ɫ����
				1                           // Mip�㼶��
			);
		}
	}

	// ����֡����������Ҫ��Ⱦͨ������
	void SwapChain::createFrameBuffers(const RenderPass::Ptr& renderPass) {
		//����FrameBuffer
		mSwapChainFrameBuffers.resize(mImageCount);
		for (int i = 0; i < mImageCount; ++i) {
			//FrameBuffer ����Ϊһ֡�����ݣ�������n��ColorAttachment 1��DepthStencilAttachment��
			//��Щ�����ļ���Ϊһ��FrameBuffer��������ߣ��ͻ��γ�һ��GPU�ļ��ϣ����Ϸ���Attachments����
			std::array<VkImageView, 1> attachments = { mSwapChainImageViews[i] };

			VkFramebufferCreateInfo frameBufferCreateInfo{};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.renderPass = renderPass->getRenderPass();
			frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferCreateInfo.pAttachments = attachments.data();
			frameBufferCreateInfo.width = mSwapChainExtent.width;
			frameBufferCreateInfo.height = mSwapChainExtent.height;
			frameBufferCreateInfo.layers = 1;

			if (vkCreateFramebuffer(mDevice->getDevice(), &frameBufferCreateInfo, nullptr, &mSwapChainFrameBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Error:Failed to create frameBuffer");
			}
		}
	}

	// ��������������������Դ
	SwapChain::~SwapChain()
	{
		for (auto& imageView : mSwapChainImageViews)
		{
			vkDestroyImageView(mDevice->getDevice(), imageView, nullptr);
		}

		for (auto& frameBuffer : mSwapChainFrameBuffers) {
			vkDestroyFramebuffer(mDevice->getDevice(), frameBuffer, nullptr);
		}

		// 3. ���ٽ���������
		if (mSwapChain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);
		}

		// 4. �ͷ�����ָ�������Դ
		mWindow.reset();
		mSurface.reset();
		mDevice.reset();
	}

	// ��ѯ�����豸�Ľ�����֧����Ϣ
	SwapChainSupportInfo SwapChain::querySwapChainSupportInfo()
	{
		SwapChainSupportInfo info;

		// 1. ��ȡ�����������
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &info.mCapabilities);

		// 2. ��ȡ֧�ֵı����ʽ
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, nullptr);
		if (formatCount != 0)
		{
			info.mFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, info.mFormats.data());
		}

		// 3. ��ȡ֧�ֵĳ���ģʽ
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			info.mPresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, info.mPresentModes.data());
		}

		return info;
	}

	// ѡ����ѱ����ʽ������ѡ��SRGB��ʽ��
	VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		// ���1��ֻ��һ��δ�����ʽ -> ʹ��Ĭ�ϸ�ʽ
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			return { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		// ���2������ѡ��B8G8R8A8_SRGB��ʽ
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		// ���3��û�������ʽ -> ���ص�һ�����ø�ʽ
		return availableFormats[0];
	}

	// ѡ����ѳ���ģʽ������ѡ������ģʽ��
	VkPresentModeKHR SwapChain::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresenstModes)
	{
		// Ĭ��ʹ��FIFO����ֱͬ���������豸֧�֣�
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		// ����ѡ������ģʽ�����ػ��壬��˺�ѣ�
		for (const auto& availablePresentMode : availablePresenstModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return bestMode;  // ����Ĭ��ģʽ
	}

	// ѡ�񽻻���ͼ��ߴ磨�ֱ��ʣ�
	VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		// ���1����ǰ�ߴ���Ч -> ֱ��ʹ��
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		// ���2����Ҫ�ֶ�ȷ���ߴ�
		int width = 0, height = 0;
		glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		// ȷ���ߴ�������Χ��
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

	// ����ͼ����ͼ
	VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;                                     // Դͼ��
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;                  // 2D��ͼ
		viewInfo.format = format;                                   // ͼ���ʽ

		viewInfo.subresourceRange.aspectMask = aspectFlags;         // ͼ����;����ɫ/��ȣ�
		viewInfo.subresourceRange.baseMipLevel = 0;                 // ��ʼMip�㼶
		viewInfo.subresourceRange.levelCount = mipLevels;           // Mip�㼶��
		viewInfo.subresourceRange.baseArrayLayer = 0;               // ��ʼ�����
		viewInfo.subresourceRange.layerCount = 1;                   // �������

		VkImageView imageView{ VK_NULL_HANDLE };
		if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create image view in swapchain");
		}

		return imageView;
	}
}