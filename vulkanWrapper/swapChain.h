#pragma once

#include "base.h"
#include "device.h"
#include "window.h"
#include "windowSurface.h"
#include "renderPass.h"

namespace Fire3D {

	struct SwapChainSupportInfo {
		VkSurfaceCapabilitiesKHR mCapabilities;
		std::vector<VkSurfaceFormatKHR> mFormats;
		std::vector<VkPresentModeKHR> mPresentModes;
	};

	class SwapChain {
	public:
		using Ptr = std::shared_ptr<SwapChain>;
		static Ptr create(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface) { 
			return std::make_shared<SwapChain>(device, window, surface); 
		}

		SwapChain(const Device::Ptr &device, const Window::Ptr &window, const WindowSurface::Ptr &surface);

		~SwapChain();

		SwapChainSupportInfo querySwapChainSupportInfo();

		VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

		VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR> &availablePresenstModes);

		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR & capabilities);

		void createFrameBuffers(const RenderPass::Ptr &renderPass);

	public:
		auto getFormat() const { return mSwapChainFormat; }

		auto getImageCount() const { return mImageCount; }

		auto getSwapChain() const { return m_swapchain; }

		auto getFrameBuffer(const int index) const { return mSwapChainFrameBuffers[index]; }

		auto getExtent() const { return mSwapChainExtent; }

	private:
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1);

	private:
		VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };

		VkFormat  mSwapChainFormat;
		VkExtent2D mSwapChainExtent;
		uint32_t mImageCount{ 0 };

		std::vector<VkImage> mSwapChainImages{};

		std::vector<VkImageView> mSwapChainImageViews{};

		std::vector<VkFramebuffer> mSwapChainFrameBuffers{};


		Device::Ptr m_device{ nullptr };
		Window::Ptr m_window{ nullptr };
		WindowSurface::Ptr m_surface{ nullptr };
	};
}