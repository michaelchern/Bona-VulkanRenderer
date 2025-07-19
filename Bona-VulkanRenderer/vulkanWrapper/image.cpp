#include "image.h"
#include "commandBuffer.h"
#include "buffer.h"

namespace LearnVulkan::Wrapper
{
    Image::Ptr Image::createDepthImage(const Device::Ptr& device, const int& width, const int& height)
    {
        std::vector<VkFormat> formats =
        {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        };

        VkFormat resultFormat = findSupportedFormat(device,
                                                    formats,
                                                    VK_IMAGE_TILING_OPTIMAL,
                                                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        return Image::create(device,
                             width,
                             height,
                             resultFormat,
                             VK_IMAGE_TYPE_2D,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                             VK_SAMPLE_COUNT_1_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    }

    Image::Image(const Device::Ptr &device,
                 const int& width,
                 const int& height,
                 const VkFormat& format,
                 const VkImageType& imageType,
                 const VkImageTiling& tiling,
                 const VkImageUsageFlags& usage,
                 const VkSampleCountFlagBits& sample,
                 const VkMemoryPropertyFlags& properties,
                 const VkImageAspectFlags& aspectFlags)
    {
        // 初始化成员变量
        mDevice = device;
        mLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        mWidth = width;    // 记录图像宽度
        mHeight = height;  // 记录图像高度
        mFormat = format;  // 记录图像格式

        // ---------------------------
        // 步骤 1：创建 Vulkan 图像（VkImage）
        // ---------------------------
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

        // 图像尺寸（二维图像只需设置 width 和 height，depth 固定为 1）
        imageCreateInfo.extent.width  = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth  = 1;

        imageCreateInfo.format        = format;     // 像素格式（与参数一致）
        imageCreateInfo.imageType     = imageType;  // 图像类型（如 2D/3D）
        imageCreateInfo.tiling        = tiling;     // 内存排列方式（优化或线性）
        imageCreateInfo.usage         = usage;      // 图像用途（决定后续如何使用，如渲染目标、纹理采样）
        imageCreateInfo.samples       = sample;     // 多重采样等级（影响抗锯齿）

        // 多级渐远纹理（Mipmap）层级数（此处固定为 1，不生成 Mipmap）
        imageCreateInfo.mipLevels     = 1;
        // 数组层数（适用于立方体贴图等数组图像，此处固定为 1）
        imageCreateInfo.arrayLayers   = 1;
        // 初始布局（图像创建后首次使用前的布局，未定义表示初始状态无需转换）
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // 共享模式（独占模式：仅一个队列族访问；共享模式：多个队列族共享）
        imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(mDevice->getDevice(), &imageCreateInfo, nullptr, &mImage) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create image!");
        }

        // ---------------------------
        // 步骤 2：分配图像内存
        // ---------------------------
        // 获取图像的内存需求（大小、对齐方式、支持的内存类型）
        VkMemoryRequirements memReq{};
        vkGetImageMemoryRequirements(mDevice->getDevice(), mImage, &memReq);

        // 内存分配信息结构体
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;

        // 内存类型索引（需满足图像需求的内存属性和传入的 properties 参数）
        // findMemoryType 函数通常根据 memReq.memoryTypeBits（支持的内存类型位掩码）和 properties（所需属性）查找匹配的索引
        allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

        if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mImageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to allocate memory!");
        }

        // 将分配的内存绑定到图像对象（图像必须绑定内存后才能使用）
        vkBindImageMemory(mDevice->getDevice(), mImage, mImageMemory, 0);

        // ---------------------------
        // 步骤 3：创建图像视图（VkImageView）
        // ---------------------------
        // 图像视图创建信息结构体（用于访问图像的特定子资源）
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        // 视图类型（根据图像类型选择：2D 图像用 VK_IMAGE_VIEW_TYPE_2D，3D 图像用 VK_IMAGE_VIEW_TYPE_3D）
        imageViewCreateInfo.viewType                        = imageType == VK_IMAGE_TYPE_2D ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;
        imageViewCreateInfo.format                          = format;
        imageViewCreateInfo.image                           = mImage;

        // 子资源范围（指定图像的哪些部分可通过视图访问）
        imageViewCreateInfo.subresourceRange.aspectMask     = aspectFlags;
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.levelCount     = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(mDevice->getDevice(), &imageViewCreateInfo, nullptr, &mImageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create image view!");
        }
    }

    Image::~Image()
    {
        if (mImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(mDevice->getDevice(), mImageView, nullptr);
        }

        if (mImageMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(mDevice->getDevice(), mImageMemory, nullptr);
        }

        if (mImage != VK_NULL_HANDLE)
        {
            vkDestroyImage(mDevice->getDevice(), mImage, nullptr);
        }
    } 

    uint32_t Image::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(mDevice->getPhysicalDevice(), &memProps);

        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties))
            {
                return i;
            }
        }

        throw std::runtime_error("Error: cannot find the property memory type!");
    }

    VkFormat Image::findDepthFormat(const Device::Ptr& device)
    {
        std::vector<VkFormat> formats =
        {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        };

        VkFormat resultFormat = findSupportedFormat(device,
                                                    formats,
                                                    VK_IMAGE_TILING_OPTIMAL,
                                                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        return resultFormat;
    }

    VkFormat Image::findSupportedFormat(const Device::Ptr& device,
                                       const std::vector<VkFormat>& candidates,
                                       VkImageTiling tiling,
                                       VkFormatFeatureFlags features)
    {
        for (const auto& format : candidates)
        {
            VkFormatProperties formatProps;
            vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), format, &formatProps);
            if (tiling == VK_IMAGE_TILING_LINEAR && (formatProps.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProps.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }
        throw std::runtime_error("Error: cannot find a supported image format!");
    }

    bool Image::hasStencilComponent(VkFormat format) const
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
               format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    
    void Image::setImageLayout(VkImageLayout newLayout,
                               VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask,
                               VkImageSubresourceRange subresrouceRange,
                               const CommandPool::Ptr& commandPool)
    {
        // 1. 初始化图像内存屏障（Image Memory Barrier）
        // 内存屏障用于告知Vulkan驱动：旧布局到新布局的转换需要同步哪些操作
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout           = mLayout;                  // 当前（旧）图像布局（成员变量保存的当前状态）
        imageMemoryBarrier.newLayout           = newLayout;                // 目标（新）图像布局（本次转换目标）
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 忽略队列族转移（不涉及队列所有权变更）
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 同上
        imageMemoryBarrier.image               = mImage;                   // 关联的目标图像对象
        imageMemoryBarrier.subresourceRange    = subresrouceRange;         // 需要转换布局的子资源范围

        // 2. 根据旧布局设置源访问掩码（Src Access Mask）
        // 源访问掩码指定旧布局阶段中需要对图像进行的操作类型，驱动需确保这些操作完成后再转换布局
        switch (mLayout)
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            // UNDEFINED布局：图像初始状态，无有效数据，无需等待任何旧操作（srcAccessMask=0）
            imageMemoryBarrier.srcAccessMask = 0;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // 传输目标布局：之前作为传输操作的写入目标（如vkCmdCopyBufferToImage）
            // 需等待传输写操作完成（VK_ACCESS_TRANSFER_WRITE_BIT）
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        default:
            // 其他旧布局未显式处理（可能根据需求扩展）
            break;
        }

        // 3. 根据新布局设置目标访问掩码（Dst Access Mask）
        // 目标访问掩码指定新布局阶段中图像将被如何访问，驱动需确保转换完成后允许这些操作
        switch (newLayout)
        {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // 转换为传输目标布局：后续操作可能是传输写（如再次复制数据）
            // 需允许传输写操作访问（VK_ACCESS_TRANSFER_WRITE_BIT）
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // 转换为着色器只读布局：后续着色器阶段将读取图像（如采样器采样）
            // 若旧布局未指定源访问（如从UNDEFINED转换），需兼容主机写或传输写的完成
            if (imageMemoryBarrier.srcAccessMask == 0)
            {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            // 允许着色器读取（VK_ACCESS_SHADER_READ_BIT）
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // 转换为深度/模板附件布局：后续渲染流程将作为深度/模板附件使用
            // 需允许深度/模板附件写操作（如渲染通道中的写入）
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        default:
            // 其他新布局未显式处理（可能根据需求扩展）
            break;
        }

        // 4. 更新图像的当前布局状态（成员变量记录最新布局）
        mLayout = newLayout;

        // 5. 创建并录制命令缓冲以提交布局转换
        // 使用指定命令池创建临时命令缓冲（用于本次一次性提交）
        auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
        // 开始命令缓冲录制（ONE_TIME_SUBMIT_BIT：仅提交一次后失效）
        commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // 6. 在命令缓冲中记录布局转换操作
        // 调用自定义方法将内存屏障提交到命令缓冲，驱动将根据src/dstStageMask同步阶段
        commandBuffer->transferImageLayout(imageMemoryBarrier, srcStageMask, dstStageMask);

        // 7. 结束命令缓冲录制
        commandBuffer->end();

        // 8. 同步提交命令缓冲到图形队列执行
        // 提交后队列会执行布局转换，等待完成后返回（同步操作，阻塞当前线程）
        commandBuffer->submitSync(mDevice->getGraphicQueue());
    }

    void Image::fillImageData(size_t size,
                              void* pData,
                              const CommandPool::Ptr& commandPool)
    {
        assert(pData);
        assert(size);

        auto stageBuffer = Buffer::createStageBuffer(mDevice, size, pData);

        auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
        commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
        commandBuffer->copyBufferToImage(stageBuffer->getBuffer(),
                                         mImage,
                                         mLayout,
                                         mWidth,
                                         mHeight);

        commandBuffer->end();

        commandBuffer->submitSync(mDevice->getGraphicQueue());
    }
}
