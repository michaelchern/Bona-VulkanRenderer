
#include "swapChain.h"

namespace LearnVulkan::Wrapper
{
    // 构造函数：创建交换链及相关资源
    SwapChain::SwapChain(const Device::Ptr& device, 
                         const Window::Ptr& window, 
                         const WindowSurface::Ptr& surface,
                         const CommandPool::Ptr& commandPool)
    {
        mDevice = device;
        mWindow = window;    // 保存窗口引用
        mSurface = surface;  // 保存表面引用

        // 1. 查询交换链支持信息
        auto swapChainSupportInfo = querySwapChainSupportInfo();

        // 2. 选择最佳表面格式（颜色格式+色彩空间）
        VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.mFormats);

        // 3. 选择最佳呈现模式（垂直同步/三重缓冲等）
        VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.mPresentModes);

        // 4. 选择交换链图像尺寸（分辨率）
        VkExtent2D extent = chooseExtent(swapChainSupportInfo.mCapabilities);

        // 5. 确定图像缓冲数量（双缓冲/三缓冲）
        mImageCount = swapChainSupportInfo.mCapabilities.minImageCount + 1;  // 多一个缓冲减少等待

        // 6. 检查最大图像数量限制（0表示无限制）
        if (swapChainSupportInfo.mCapabilities.maxImageCount > 0 && mImageCount > swapChainSupportInfo.mCapabilities.maxImageCount)
        {
            mImageCount = swapChainSupportInfo.mCapabilities.maxImageCount;
        }

        // 7. 填写交换链创建信息
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;  
        createInfo.surface = mSurface->getSurface();                     // 绑定窗口表面
        createInfo.minImageCount = mImageCount;                          // 最小缓冲数量
        createInfo.imageFormat = surfaceFormat.format;                   // 图像格式
        createInfo.imageColorSpace = surfaceFormat.colorSpace;           // 色彩空间
        createInfo.imageExtent = extent;                                 // 图像尺寸
        createInfo.imageArrayLayers = 1;                                 // 图像层次数（非VR应用为1）
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;     // 图像用途（颜色附件）

        // 8. 处理队列族共享模式
        std::vector<uint32_t> queueFamilies = { mDevice->getGraphicQueueFamily().value() , mDevice->getPresentQueueFamily().value() };

        // 8.1 图形队列和呈现队列相同时 - 独占模式（性能更优）
        if (mDevice->getGraphicQueueFamily().value() == mDevice->getPresentQueueFamily().value())
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;                     // 不需要指定队列族
            createInfo.pQueueFamilyIndices = nullptr;
        }
        else  // 8.2 不同队列族 - 并发模式（需要共享）
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
            createInfo.pQueueFamilyIndices = queueFamilies.data();
        }

        // 9. 设置其他参数
        createInfo.preTransform = swapChainSupportInfo.mCapabilities.currentTransform;  // 图像变换
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                  // 不混合Alpha通道
        createInfo.presentMode = presentMode;                                           // 呈现模式
        createInfo.clipped = VK_TRUE;                                                   // 裁剪被遮挡部分（提高性能）

        // 10. 创建交换链
        if (vkCreateSwapchainKHR(mDevice->getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create swapChain!");
        }

        // 11. 保存关键参数
        mSwapChainFormat = surfaceFormat.format;
        mSwapChainExtent = extent;

        // 12. 获取交换链实际创建的图像数量（可能多于请求）
        vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, nullptr);
        mSwapChainImages.resize(mImageCount);
        vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, mSwapChainImages.data());

        // 13. 为每个交换链图像创建图像视图
        mSwapChainImageViews.resize(mImageCount);
        for (int i = 0; i < mImageCount; ++i)
        {
            mSwapChainImageViews[i] = createImageView(mSwapChainImages[i],
                                                      mSwapChainFormat,
                                                      VK_IMAGE_ASPECT_COLOR_BIT,  // 作为颜色附件
                                                      1);                         // Mip层级数（1表示无Mipmapping）
        }

        mDepthImages.resize(mImageCount);

        VkImageSubresourceRange region{};
        region.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        region.baseMipLevel   = 0;
        region.levelCount     = 1;
        region.baseArrayLayer = 0;
        region.layerCount     = 1;

        for (int i = 0; i < mImageCount; ++i)
        {
            mDepthImages[i] = Image::createDepthImage(mDevice, mSwapChainExtent.width, mSwapChainExtent.height);
            mDepthImages[i]->setImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                            region,
                                            commandPool);
        }
    }

    // 创建帧缓冲区（需要渲染通道对象）
    void SwapChain::createFrameBuffers(const RenderPass::Ptr& renderPass)
    {
        // 初始化帧缓冲区容器
        mSwapChainFrameBuffers.resize(mImageCount);

        // 为每个交换链图像创建帧缓冲区
        for (int i = 0; i < mImageCount; ++i)
        {
            //FrameBuffer 里面为一帧的数据，比如有n个ColorAttachment 1个DepthStencilAttachment，
            //这些东西的集合为一个FrameBuffer，送入管线，就会形成一个GPU的集合，由上方的Attachments构成
            std::array<VkImageView, 2> attachments = { mSwapChainImageViews[i] ,mDepthImages[i]->getImageView() };

            VkFramebufferCreateInfo frameBufferCreateInfo{};
            frameBufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferCreateInfo.renderPass      = renderPass->getRenderPass();                
            frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferCreateInfo.pAttachments    = attachments.data();                         
            frameBufferCreateInfo.width           = mSwapChainExtent.width;                     
            frameBufferCreateInfo.height          = mSwapChainExtent.height;
            frameBufferCreateInfo.layers          = 1;

            // 创建帧缓冲区
            if (vkCreateFramebuffer(mDevice->getDevice(), &frameBufferCreateInfo, nullptr, &mSwapChainFrameBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Error:Failed to create frameBuffer!");
            }
        }
    }

    // 析构函数：按正确顺序清理所有资源
    SwapChain::~SwapChain()
    {
        // 1. 销毁所有图像视图
        for (auto& imageView : mSwapChainImageViews)
        {
            vkDestroyImageView(mDevice->getDevice(), imageView, nullptr);
        }

        // 2. 销毁所有帧缓冲区
        for (auto& frameBuffer : mSwapChainFrameBuffers)
        {
            vkDestroyFramebuffer(mDevice->getDevice(), frameBuffer, nullptr);
        }

        // 3. 销毁交换链本体
        if (mSwapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);
        }

        // 4. 释放智能指针管理资源（按依赖顺序）
        mWindow.reset();
        mSurface.reset();  // 表面依赖于设备
        mDevice.reset();   // 设备最后释放
    }

    // 查询物理设备的交换链支持信息
    SwapChainSupportInfo SwapChain::querySwapChainSupportInfo()
    {
        SwapChainSupportInfo info;

        // 1. 获取表面基本能力
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &info.mCapabilities);

        // 2. 查询支持的表面格式
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, nullptr);
        if (formatCount != 0)
        {
            info.mFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, info.mFormats.data());
        }

        // 3. 查询支持的呈现模式
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            info.mPresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, info.mPresentModes.data());
        }

        return info;
    }

    // 选择最佳表面格式（优先选择SRGB格式）
    VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        // 情况1: 只有一个格式且未定义 -> 使用默认设置
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            return { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        }

        // 情况2: 寻找最佳格式：B8G8R8A8_SRGB + SRGB非线性色彩空间
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        // 情况3: 没有理想格式 -> 返回第一个可用格式
        return availableFormats[0];
    }

    // 选择最佳呈现模式（优先选择邮箱模式）
    VkPresentModeKHR SwapChain::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresenstModes)
    {
        // 默认使用FIFO（垂直同步，所有设备支持）
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        // 优先选择邮箱模式（三重缓冲，无撕裂）
        for (const auto& availablePresentMode : availablePresenstModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return bestMode;  // 返回默认模式
    }

    // 选择交换链图像尺寸（分辨率）
    VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        // 情况1：当前尺寸有效 -> 直接使用
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        // 情况2：需要手动确定尺寸
        int width = 0, height = 0;
        glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // 确保尺寸在允许范围内
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }

    // 创建图像视图
    VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;                                     // 源图像
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;                  // 2D视图
        viewInfo.format = format;                                   // 图像格式

        // 配置子资源范围
        viewInfo.subresourceRange.aspectMask = aspectFlags;         // 图像用途（颜色/深度）
        viewInfo.subresourceRange.baseMipLevel = 0;                 // 起始Mip层级
        viewInfo.subresourceRange.levelCount = mipLevels;           // Mip层级数
        viewInfo.subresourceRange.baseArrayLayer = 0;               // 起始数组层
        viewInfo.subresourceRange.layerCount = 1;                   // 数组层数

        VkImageView imageView{ VK_NULL_HANDLE };
        if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create image view in swapchain");
        }

        return imageView;
    }
}