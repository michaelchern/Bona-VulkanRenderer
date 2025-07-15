#pragma once

#include "base.h"
#include "device.h"
#include "window.h"
#include "windowSurface.h"
#include "renderPass.h"
#include "image.h"
#include "commandPool.h"

namespace LearnVulkan::Wrapper
{
    struct SwapChainSupportInfo
    {
        VkSurfaceCapabilitiesKHR        mCapabilities;
        std::vector<VkSurfaceFormatKHR> mFormats;
        std::vector<VkPresentModeKHR>   mPresentModes;
    };

    class SwapChain
    {
    public:
        using Ptr = std::shared_ptr<SwapChain>;
        static Ptr create(const Device::Ptr& device,
                          const Window::Ptr& window,
                          const WindowSurface::Ptr& surface,
                          const CommandPool::Ptr& commandPool)
        {
            return std::make_shared<SwapChain>(device, window, surface, commandPool);
        }

        // SwapChain 类构造函数：负责创建 Vulkan 交换链（Swap Chain）及相关资源
        // 参数说明：
        // - device: Vulkan 逻辑设备指针，用于与 GPU 交互
        // - window: 窗口对象指针，提供窗口系统相关接口（如 GLFW 窗口）
        // - surface: Vulkan 窗口表面指针，连接窗口系统与 Vulkan 的桥梁（用于呈现图像到屏幕）
        // - commandPool: 命令池指针，用于分配 Vulkan 命令缓冲区（此处用于深度图像布局转换）
        SwapChain(const Device::Ptr& device, 
                  const Window::Ptr& window, 
                  const WindowSurface::Ptr& surface,
                  const CommandPool::Ptr& commandPool);

        ~SwapChain();

        SwapChainSupportInfo querySwapChainSupportInfo();

        VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresenstModes);

        VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        void createFrameBuffers(const RenderPass::Ptr& renderPass);

    public:

        [[nodiscard]] auto getFormat() const { return mSwapChainFormat; }

        [[nodiscard]] auto getImageCount() const { return mImageCount; }

        [[nodiscard]] auto getSwapChain() const { return mSwapChain; }

        [[nodiscard]] auto getFrameBuffer(const int index) const { return mSwapChainFrameBuffers[index]; }

        [[nodiscard]] auto getExtent() const { return mSwapChainExtent; }

    private:

        // 辅助函数：为 Vulkan 图像创建图像视图（Image View）
        // 图像视图是 Vulkan 中访问图像的核心方式，用于指定如何解释图像的格式、子资源（如 Mipmap 层级、数组层）等
        // 参数说明：
        // - image: 待创建视图的 Vulkan 图像对象（如交换链中的颜色图像或深度图像）
        // - format: 图像的像素格式（需与图像本身的格式一致，如 VK_FORMAT_B8G8R8A8_SRGB）
        // - aspectFlags: 图像的方面标志（指定视图的访问类型，如颜色通道、深度通道等）
        //   常见值：VK_IMAGE_ASPECT_COLOR_BIT（颜色图像）、VK_IMAGE_ASPECT_DEPTH_BIT（深度图像）
        // - mipLevels: 图像的 Mipmap 层级数量（通常为 1，若图像生成了 Mipmap 则为实际层级数）
        VkImageView createImageView(VkImage image,
                                    VkFormat format,
                                    VkImageAspectFlags aspectFlags,
                                    uint32_t mipLevels = 1);

    private:

        VkFormat   mSwapChainFormat;
        VkExtent2D mSwapChainExtent;
        uint32_t   mImageCount{ 0 };

        std::vector<VkImage>       mSwapChainImages{};
        std::vector<VkImageView>   mSwapChainImageViews{};
        std::vector<VkFramebuffer> mSwapChainFrameBuffers{};
        std::vector<Image::Ptr>    mDepthImages{};

        Device::Ptr        mDevice{ nullptr };
        Window::Ptr        mWindow{ nullptr };
        WindowSurface::Ptr mSurface{ nullptr };
        VkSwapchainKHR     mSwapChain{ VK_NULL_HANDLE };
    };
}
