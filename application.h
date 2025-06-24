#pragma once

// 包含Vulkan封装层各个模块的头文件
#include "vulkanWrapper/base.h"                 // Vulkan基础封装
#include "vulkanWrapper/instance.h"             // Vulkan实例封装
#include "vulkanWrapper/device.h"               // 逻辑设备封装
#include "vulkanWrapper/window.h"               // 窗口管理封装
#include "vulkanWrapper/windowSurface.h"        // 窗口表面封装
#include "vulkanWrapper/swapChain.h"            // 交换链封装
#include "vulkanWrapper/shader.h"               // 着色器封装
#include "vulkanWrapper/pipeline.h"             // 图形管线封装
#include "vulkanWrapper/renderPass.h"           // 渲染通道封装
#include "vulkanWrapper/commandPool.h"          // 命令池封装
#include "vulkanWrapper/commandBuffer.h"        // 命令缓冲区封装
#include "vulkanWrapper/semaphore.h"            // 信号量封装
#include "vulkanWrapper/fence.h"                // 栅栏封装
#include "vulkanWrapper/buffer.h"               // 缓冲区封装
#include "vulkanWrapper/descriptorSetLayout.h"  // 描述符集布局封装
#include "vulkanWrapper/descriptorPool.h"       // 描述符池封装
#include "vulkanWrapper/descriptorSet.h"        // 描述符集封装
#include "vulkanWrapper/description.h"          // 描述符相关工具
#include "uniformManager.h"                     // 统一缓冲区管理器
#include "vulkanWrapper/image.h"                // 图像资源封装
#include "vulkanWrapper/sampler.h"              // 采样器封装
#include "texture/texture.h"                    // 纹理管理

#include "model.h"                              // 模型加载与渲染

namespace LearnVulkan
{
    /**
     * @brief Vulkan应用程序主类
     *
     * 整合Vulkan渲染管线的核心组件，管理应用程序生命周期：
     * 1. 初始化窗口和Vulkan资源
     * 2. 主循环处理事件和渲染
     * 3. 资源清理
     */
    class Application
    {
    public:
        Application() = default;   // 默认构造函数
        ~Application() = default;  // 默认析构函数

        /// 运行应用程序主入口
        void run();

    private:
        // 初始化阶段方法
        void initWindow();            // 初始化GLFW窗口
        void initVulkan();            // 初始化Vulkan核心对象
        void createPipeline();        // 创建图形管线
        void createRenderPass();      // 创建渲染通道
        void createCommandBuffers();  // 创建命令缓冲区
        void createSyncObjects();     // 创建同步对象（信号量/栅栏）

        /// 主循环控制
        void mainLoop();

        /// 单帧渲染逻辑
        void render();

        /// 窗口大小改变时重建交换链
        void recreateSwapChain();

        /// 清理交换链相关资源
        void cleanupSwapChain();

        /// 应用程序退出时清理所有资源
        void cleanUp();

    private:
        unsigned int mWidth{ 800 };   // 窗口宽度
        unsigned int mHeight{ 600 };  // 窗口高度

    private:
        // Vulkan核心对象
        int                         mCurrentFrame{ 0 };      // 当前帧索引（用于多帧同步）
        Wrapper::Window::Ptr        mWindow{ nullptr };      // GLFW窗口管理
        Wrapper::Instance::Ptr      mInstance{ nullptr };    // Vulkan实例
        Wrapper::Device::Ptr        mDevice{ nullptr };      // 逻辑设备
        Wrapper::WindowSurface::Ptr mSurface{ nullptr };     // 窗口表面
        Wrapper::SwapChain::Ptr     mSwapChain{ nullptr };   // 交换链管理
        Wrapper::Pipeline::Ptr      mPipeline{ nullptr };    // 图形管线
        Wrapper::RenderPass::Ptr    mRenderPass{ nullptr };  // 渲染通道

        // 命令管理
        Wrapper::CommandPool::Ptr                mCommandPool{ nullptr };  // 命令池
        std::vector<Wrapper::CommandBuffer::Ptr> mCommandBuffers{};        // 每帧命令缓冲区

        // 同步对象（多帧并发）
		std::vector<Wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};  // 图像可用信号量，图片显示完毕后才允许渲染
		std::vector<Wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};  // 渲染完成信号量，渲染完成后才允许交换链更新
        std::vector<Wrapper::Fence::Ptr> mFences{};                        // 帧同步栅栏

        // 资源管理
        UniformManager::Ptr mUniformManager{ nullptr };  // 统一变量缓冲区管理
        Model::Ptr          mModel{ nullptr };           // 渲染模型
        VPMatrices          mVPMatrices;                 // 视图/投影矩阵数据
    };
}