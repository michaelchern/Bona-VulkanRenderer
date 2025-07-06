#pragma once

#include "base.h"
#include "device.h"
#include "window.h"
#include "windowSurface.h"
#include "renderPass.h"

namespace LearnVulkan::Wrapper
{
    // 交换链支持信息结构体
    struct SwapChainSupportInfo
    {
        VkSurfaceCapabilitiesKHR mCapabilities;       // 表面能力（最小/最大图像数量，图像尺寸范围等）
        std::vector<VkSurfaceFormatKHR> mFormats;     // 支持的像素格式列表
        std::vector<VkPresentModeKHR> mPresentModes;  // 支持的呈现模式列表
    };

    // Vulkan 交换链管理类
    class SwapChain
    {
    public:
        using Ptr = std::shared_ptr<SwapChain>;
        static Ptr create(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface)
        {
            return std::make_shared<SwapChain>(device, window, surface);
        }

        // 构造函数：创建交换链及相关资源
        SwapChain(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface);

        // 析构函数：销毁交换链资源
        ~SwapChain();

        // 查询当前物理设备的交换链支持信息
        SwapChainSupportInfo querySwapChainSupportInfo();

        // 从可用格式中选择最佳表面格式
        VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        // 从可用模式中选择最佳呈现模式
        VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresenstModes);

        // 选择交换链图像尺寸（分辨率）
        VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        // 为交换链创建帧缓冲区（依赖渲染通道）
        void createFrameBuffers(const RenderPass::Ptr& renderPass);

    public:
        // === 访问方法 ===

        [[nodiscard]] auto getFormat() const { return mSwapChainFormat; }                                   // 获取交换链图像格式

        [[nodiscard]] auto getImageCount() const { return mImageCount; }                                    // 获取交换链图像数量

        [[nodiscard]] auto getSwapChain() const { return mSwapChain; }                                      // 获取底层VkSwapchainKHR句柄

        [[nodiscard]] auto getFrameBuffer(const int index) const { return mSwapChainFrameBuffers[index]; }  // 获取指定帧缓冲区

        [[nodiscard]] auto getExtent() const { return mSwapChainExtent; }                                   // 获取交换链图像尺寸

    private:
        // 创建图像视图（用于访问图像）
        VkImageView createImageView(VkImage image,
                                    VkFormat format,
                                    VkImageAspectFlags aspectFlags,
                                    uint32_t mipLevels = 1);

    private:
        
        VkSwapchainKHR mSwapChain{ VK_NULL_HANDLE };          // Vulkan交换链对象句柄

        VkFormat  mSwapChainFormat;                           // 交换链图像格式（如RGBA8）
        VkExtent2D mSwapChainExtent;                          // 交换链图像尺寸（宽度x高度）
        uint32_t mImageCount{ 0 };                            // 交换链中的图像数量

        // 由交换链管理的图像（不需要手动销毁）
        std::vector<VkImage> mSwapChainImages{};

        // 图像视图：提供对交换链图像的访问接口
        std::vector<VkImageView> mSwapChainImageViews{};

        // 帧缓冲区：连接渲染通道和图像视图
        std::vector<VkFramebuffer> mSwapChainFrameBuffers{};

        // 依赖资源
        Device::Ptr mDevice{ nullptr };                       // 逻辑设备引用
        Window::Ptr mWindow{ nullptr };                       // 窗口引用（用于获取尺寸）
        WindowSurface::Ptr mSurface{ nullptr };               // 窗口表面引用
    };
}