
#include "swapChain.h"

namespace LearnVulkan::Wrapper
{
    SwapChain::SwapChain(const Device::Ptr& device, 
                         const Window::Ptr& window, 
                         const WindowSurface::Ptr& surface,
                         const CommandPool::Ptr& commandPool)
    {
        mDevice  = device;
        mWindow  = window;
        mSurface = surface;

        // 步骤1：查询物理设备对交换链的支持信息
        // 包括支持的图像格式、呈现模式、交换链能力（如最大/最小图像数、当前变换等）
        auto swapChainSupportInfo = querySwapChainSupportInfo();

        // 步骤2：选择交换链的表面格式（颜色空间+像素格式）
        // 例如：VK_FORMAT_B8G8R8A8_SRGB（常见 sRGB 格式）搭配 VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.mFormats);

        // 步骤3：选择呈现模式（控制图像如何呈现到屏幕）
        // 常见模式：VK_PRESENT_MODE_MAILBOX_KHR（无撕裂，低延迟）、VK_PRESENT_MODE_FIFO_KHR（垂直同步）
        VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.mPresentModes);

        // 步骤4：确定交换链的分辨率（Extent）
        // 优先使用窗口请求的尺寸，若超出设备支持范围则使用设备推荐值
        VkExtent2D extent = chooseExtent(swapChainSupportInfo.mCapabilities);

        // 步骤5：确定交换链的图像数量（双缓冲/三缓冲）
        // 最小图像数 + 1 通常用于双缓冲（避免渲染与呈现冲突）
        mImageCount = swapChainSupportInfo.mCapabilities.minImageCount + 1;

        // 限制图像数不超过设备支持的最大值（若设备限制了最大值）
        if (swapChainSupportInfo.mCapabilities.maxImageCount > 0 && mImageCount > swapChainSupportInfo.mCapabilities.maxImageCount)
        {
            mImageCount = swapChainSupportInfo.mCapabilities.maxImageCount;
        }

        // 步骤6：填充交换链创建信息结构体（VkSwapchainCreateInfoKHR）
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;  
        createInfo.surface          = mSurface->getSurface();               // 关联的窗口表面
        createInfo.minImageCount    = mImageCount;                          // 交换链图像数量
        createInfo.imageFormat      = surfaceFormat.format;                 // 表面像素格式（如 B8G8R8A8）
        createInfo.imageColorSpace  = surfaceFormat.colorSpace;             // 表面颜色空间（如 sRGB）
        createInfo.imageExtent      = extent;                               // 交换链图像分辨率（宽×高）
        createInfo.imageArrayLayers = 1;                                    // 每个图像的层数（2D 图像为 1）
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // 图像用途：作为颜色附件（渲染目标）

        // 步骤7：配置图像共享模式（多队列访问控制）
        // 获取图形队列和呈现队列的家族索引（可能相同或不同）
        std::vector<uint32_t> queueFamilies = { mDevice->getGraphicQueueFamily().value() , mDevice->getPresentQueueFamily().value() };

        if (mDevice->getGraphicQueueFamily().value() == mDevice->getPresentQueueFamily().value())
        {
            // 若图形队列和呈现队列属于同一家族：使用独占模式（更高效）
            // 独占模式下，同一时间仅一个队列可访问图像
            createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices   = nullptr;
        }
        else
        {
            // 若属于不同家族：使用并发模式（需同时指定两个家族）
            // 并发模式下，两个队列可同时访问图像（可能降低性能但更灵活）
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
            createInfo.pQueueFamilyIndices   = queueFamilies.data();
        }

        // 其他交换链配置参数
        createInfo.preTransform   = swapChainSupportInfo.mCapabilities.currentTransform;  // 屏幕变换（如旋转/翻转，使用设备当前设置）
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                    // 不透明合成（忽略 alpha 通道）
        createInfo.presentMode    = presentMode;                                          // 选择的呈现模式
        createInfo.clipped        = VK_TRUE;                                              // 允许裁剪（避免窗口遮挡时的渲染浪费）

        // 步骤8：创建 Vulkan 交换链对象
        // 调用 Vulkan API 创建交换链，若失败则抛出异常
        if (vkCreateSwapchainKHR(mDevice->getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create swapChain!");
        }

        // 步骤9：保存交换链的格式和分辨率（后续渲染需要）
        mSwapChainFormat = surfaceFormat.format;
        mSwapChainExtent = extent;

        // 步骤10：获取交换链的所有图像对象
        // 首次调用获取实际图像数量（可能因驱动调整，虽理论上应等于 mImageCount）
        vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, nullptr);
        // 调整容器大小并获取所有图像
        mSwapChainImages.resize(mImageCount);
        vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, mSwapChainImages.data());

        // 步骤11：为每个交换链图像创建图像视图（ImageView）
        // Vulkan 中图像无法直接使用，需通过视图指定格式、子资源等访问方式
        mSwapChainImageViews.resize(mImageCount);
        for (int i = 0; i < mImageCount; ++i)
        {
            // 创建颜色图像视图（用于后续渲染管线的附件绑定）
            mSwapChainImageViews[i] = createImageView(mSwapChainImages[i],        // 目标图像对象
                                                      mSwapChainFormat,           // 图像格式（与交换链一致）
                                                      VK_IMAGE_ASPECT_COLOR_BIT,  // 子资源方面：颜色通道
                                                      1);                         // 子资源层级数（单层）
        }

        // 步骤12：创建深度图像（用于深度测试）
        // 每个交换链图像需要对应的深度图像（同步创建，保证与交换链图像一一对应）
        //mDepthImages.resize(mImageCount);

        // 定义深度图像的子资源范围（仅深度通道，单层）
        //VkImageSubresourceRange region{};
        //region.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;  // 仅深度通道
        //region.baseMipLevel   = 0;                          // 基础 Mipmap 层级（无 Mipmap 时为 0）
        //region.levelCount     = 1;                          // Mipmap 层级数（单层）
        //region.baseArrayLayer = 0;                          // 基础数组层（单层图像为 0）
        //region.layerCount     = 1;                          // 数组层数（单层）

        // 循环创建每个深度图像并设置布局
        //for (int i = 0; i < mImageCount; ++i)
        //{
        //    // 创建深度图像对象（格式通常为 VK_FORMAT_D32_SFLOAT 或 VK_FORMAT_D24_UNORM_S8_UINT）
        //    mDepthImages[i] = Image::createDepthImage(mDevice, mSwapChainExtent.width, mSwapChainExtent.height);

        //    // 设置深度图像的初始布局（从 "未定义" 转换为 "深度模板附件最优"）
        //    // 使图像准备好用于深度测试阶段
        //    mDepthImages[i]->setImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        //                                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        //                                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        //                                    region,
        //                                    commandPool);
        //}
    }

    void SwapChain::createFrameBuffers(const RenderPass::Ptr& renderPass)
    {
        mSwapChainFrameBuffers.resize(mImageCount);

        for (int i = 0; i < mImageCount; ++i)
        {
            //FrameBuffer 里面为一帧的数据，比如有n个ColorAttachment 1个DepthStencilAttachment，
            //这些东西的集合为一个FrameBuffer，送入管线，就会形成一个GPU的集合，由上方的Attachments构成
            std::array<VkImageView, 1> attachments = { mSwapChainImageViews[i] };

            VkFramebufferCreateInfo frameBufferCreateInfo{};
            frameBufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferCreateInfo.renderPass      = renderPass->getRenderPass();                
            frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferCreateInfo.pAttachments    = attachments.data();                         
            frameBufferCreateInfo.width           = mSwapChainExtent.width;                     
            frameBufferCreateInfo.height          = mSwapChainExtent.height;
            frameBufferCreateInfo.layers          = 1;

            if (vkCreateFramebuffer(mDevice->getDevice(), &frameBufferCreateInfo, nullptr, &mSwapChainFrameBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Error: failed to create frameBuffer!");
            }
        }
    }

    SwapChain::~SwapChain()
    {
        for (auto& imageView : mSwapChainImageViews)
        {
            vkDestroyImageView(mDevice->getDevice(), imageView, nullptr);
        }

        for (auto& frameBuffer : mSwapChainFrameBuffers)
        {
            vkDestroyFramebuffer(mDevice->getDevice(), frameBuffer, nullptr);
        }

        if (mSwapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);
        }

        mWindow.reset();
        mSurface.reset();
        mDevice.reset();
    }

    SwapChainSupportInfo SwapChain::querySwapChainSupportInfo()
    {
        SwapChainSupportInfo info;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &info.mCapabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, nullptr);
        if (formatCount != 0)
        {
            info.mFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, info.mFormats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            info.mPresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, info.mPresentModes.data());
        }

        return info;
    }

    VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            return { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        }

        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresenstModes)
    {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto& availablePresentMode : availablePresenstModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return bestMode;
    }

    VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        int width = 0, height = 0;
        glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }

    VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
    {
        // 步骤1：填充图像视图创建信息结构体（VkImageViewCreateInfo）
        // 该结构体定义了如何从原始图像中创建一个可被着色器或其他操作访问的“视图”
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image    = image;                     // 关联的原始图像对象（要创建视图的图像）
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;    // 图像视图类型：2D 图像（交换链图像通常为 2D）

        // 指定视图的像素格式（必须与图像本身的格式一致，否则无法正确解析像素数据）
        viewInfo.format   = format;

        // 子资源范围（Subresource Range）：定义视图中包含的图像子资源（如 Mipmap 层级、数组层等）
        viewInfo.subresourceRange.aspectMask     = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        VkImageView imageView{ VK_NULL_HANDLE };

        if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create image view in swapchain!");
        }

        return imageView;
    }
}
