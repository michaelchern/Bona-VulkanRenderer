
#include "image.h"
#include "commandBuffer.h"
#include "buffer.h"

namespace LearnVulkan::Wrapper
{
    // 构造函数实现
    Image::Image(
        const Device::Ptr &device,                // Vulkan逻辑设备包装器
        const int& width,                         // 图像宽度（像素）
        const int& height,                        // 图像高度（像素）
        const VkFormat& format,                   // 像素格式（如VK_FORMAT_R8G8B8A8_SRGB）
        const VkImageType& imageType,             // 图像类型（2D/3D）
        const VkImageTiling& tiling,              // 内存排布方式（线性/最优）
        const VkImageUsageFlags& usage,           // 图像用途标志（传输目标/采样等）
        const VkSampleCountFlagBits& sample,      // 多重采样数
        const VkMemoryPropertyFlags& properties,  // 内存属性要求（设备本地等）
        const VkImageAspectFlags& aspectFlags)    // 图像视图的观察方向（颜色/深度等）
    {
        // 初始化成员变量
        mDevice = device;
        mLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // 初始设置为未定义布局
        mWidth = width;
        mHeight = height;

        // 设置图像创建信息结构体
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.extent.width  = width;                      // 图像宽度
        imageCreateInfo.extent.height = height;                     // 图像高度
        imageCreateInfo.extent.depth  = 1;                          // 深度为1（2D图像）
        imageCreateInfo.format        = format;                     // 像素格式
        imageCreateInfo.imageType     = imageType;                  // 图像类型（一般为2D）
        imageCreateInfo.tiling        = tiling;                     // 内存排布方式（最优布局利于GPU访问）
        imageCreateInfo.usage         = usage;                      // 使用场景（传输目标+采样器）
        imageCreateInfo.samples       = sample;                     // 多重采样数（默认单采样）
        imageCreateInfo.mipLevels     = 1;                          // Mipmap层级数（无mipmap）
        imageCreateInfo.arrayLayers   = 1;                          // 图像阵列层数（单层）
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // 初始布局
        imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;  // 独占访问模式

        // 创建Vulkan图像对象
        if (vkCreateImage(mDevice->getDevice(), &imageCreateInfo, nullptr, &mImage) != VK_SUCCESS)
        {
            throw std::runtime_error("Error:failed to create image");
        }

        // 分配并绑定图像内存空间 =================================
        // 获取图像的内存需求（大小/对齐/内存类型）
        VkMemoryRequirements memReq{};
        vkGetImageMemoryRequirements(mDevice->getDevice(), mImage, &memReq);

        // 配置内存分配信息
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;  // 所需内存大小

        // 符合我上述buffer需求的内存类型的ID们！0x001 0x010
        // 查找满足需求的内存类型（设备本地内存等）
        allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

        // 分配图像内存
        if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mImageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to allocate memory");
        }

        // 将内存绑定到图像对象
        vkBindImageMemory(mDevice->getDevice(), mImage, mImageMemory, 0);

        // 创建图像视图（用于着色器访问）==========================
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        // 视图类型：2D图像对应2D视图
        imageViewCreateInfo.viewType                        = imageType == VK_IMAGE_TYPE_2D ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;
        imageViewCreateInfo.format                          = format;       // 与图像格式一致
        imageViewCreateInfo.image                           = mImage;       // 绑定的图像
        // 子资源范围配置
        imageViewCreateInfo.subresourceRange.aspectMask     = aspectFlags;  // 颜色/深度
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;            // 基础Mip层级
        imageViewCreateInfo.subresourceRange.levelCount     = 1;            // Mip层级数量
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;            // 基础阵列层
        imageViewCreateInfo.subresourceRange.layerCount     = 1;            // 阵列层数

        // 创建图像视图
        if (vkCreateImageView(mDevice->getDevice(), &imageViewCreateInfo, nullptr, &mImageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Error: failed to create image view");
        }
    }

    // 析构函数实现（清理Vulkan资源）
    Image::~Image()
    {
        // 销毁图像视图
        if (mImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(mDevice->getDevice(), mImageView, nullptr);
        }

        // 释放图像内存
        if (mImageMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(mDevice->getDevice(), mImageMemory, nullptr);
        }

        // 销毁图像对象
        if (mImage != VK_NULL_HANDLE)
        {
            vkDestroyImage(mDevice->getDevice(), mImage, nullptr);
        }
    } 

    // 查找满足条件的内存类型
    uint32_t Image::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        // 获取物理设备内存属性
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(mDevice->getPhysicalDevice(), &memProps);

        // 0x001 | 0x100 = 0x101  i = 0 第i个对应类型就是  1 << i 1   i = 1 0x010
        // 遍历可用内存类型
        // typeFilter: 位掩码，每个比特位代表一个内存类型
        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
        {
            // 检查：1. 类型是否可用(typeFilter对应位为1)
            //      2. 是否满足所需属性(设备本地/主机可见等)
            if ((typeFilter & (1 << i)) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties))
            {
                return i;  // 返回符合条件的内存类型索引
            }
        }

        throw std::runtime_error("Error: cannot find the property memory type!");
    }

    // 转换图像布局（通过管线屏障）
    void Image::setImageLayout(
        VkImageLayout newLayout,                   // 目标布局
        VkPipelineStageFlags srcStageMask,         // 源操作阶段
        VkPipelineStageFlags dstStageMask,         // 目标操作阶段
        VkImageSubresourceRange subresrouceRange,  // 影响的子资源范围
        const CommandPool::Ptr& commandPool)       // 命令池（用于创建命令缓冲区）
    {
        // 设置图像内存屏障
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout           = mLayout;                  // 当前布局
        imageMemoryBarrier.newLayout           = newLayout;                // 目标布局
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 不转移队列所有权
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image               = mImage;                   // 目标图像
        imageMemoryBarrier.subresourceRange    = subresrouceRange;         // 影响的子资源范围

        // 源访问掩码：基于当前布局确定需要等待的操作
        switch (mLayout)
        {
            // 如果是无定义layout，说明图片刚被创建，上方一定没有任何操作，所以上方是一个虚拟的依赖
            // 所以不关心上一个阶段的任何操作
        case VK_IMAGE_LAYOUT_UNDEFINED:
            imageMemoryBarrier.srcAccessMask = 0;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // 当前是传输目标：确保之前的写入完成
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        // 可扩展其他布局情况...
        default:
            break;
        }

        // 目标访问掩码：基于目标布局确定后续需要什么操作权限
        switch (newLayout)
        {
            // 如果目标是，将图片转换成为一83个复制操作的目标图片/内存，那么被阻塞的操作一定是写入操作
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // 目标是传输目标：准备接收写入操作
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
            //如果目标是，将图片转换成为一个适合被作为纹理的格式，那么被阻塞的操作一定是，读取
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // 目标是着色器只读：后续需要读权限
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            break;
        }

        // 更新当前布局状态
        mLayout = newLayout;

        // 创建一次性命令缓冲区执行布局转换
        auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
        commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);  // 一次性使用标志
        // 记录布局转换命令
        commandBuffer->transferImageLayout(imageMemoryBarrier, srcStageMask, dstStageMask);
        commandBuffer->end();

        // 提交到图形队列并等待完成（同步操作）
        commandBuffer->submitSync(mDevice->getGraphicQueue());
    }

	// 填充图像数据（从主机到设备的内存传输）
    void Image::fillImageData(
        size_t size,                          // 数据字节数
        void* pData,                          // 主机端数据指针
        const CommandPool::Ptr& commandPool)  // 命令池
    {
        // 参数检查
        assert(pData);
        assert(size);

        // 1. 创建主机可见的暂存缓冲区并填充数据
        auto stageBuffer = Buffer::createStageBuffer(mDevice, size, pData);

        // 2. 创建命令缓冲区执行拷贝操作
        auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
        commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        // 从缓冲区复制到图像（需要图像已转为TRANSFER_DST布局）
        commandBuffer->copyBufferToImage(
            stageBuffer->getBuffer(),  // 源缓冲区
            mImage,                    // 目标图像
            mLayout,                   // 当前图像布局（必须是TRANSFER_DST）
            mWidth, mHeight            // 图像尺寸
        );
        commandBuffer->end();

        // 3. 提交命令并等待完成（同步操作）
        commandBuffer->submitSync(mDevice->getGraphicQueue());

        // 注意：暂存缓冲区会在作用域结束时自动销毁
    }
}