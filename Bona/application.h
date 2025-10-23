#pragma once

#include "vulkanWrapper/base.h"
#include "vulkanWrapper/instance.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/window.h"
#include "vulkanWrapper/windowSurface.h"
#include "vulkanWrapper/swapChain.h"
#include "vulkanWrapper/shader.h"
#include "vulkanWrapper/pipeline.h"
#include "vulkanWrapper/renderPass.h"
#include "vulkanWrapper/commandPool.h"
#include "vulkanWrapper/commandBuffer.h"
#include "vulkanWrapper/semaphore.h"
#include "vulkanWrapper/fence.h"
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/description.h"
#include "uniformManager.h"
#include "vulkanWrapper/image.h"
#include "vulkanWrapper/sampler.h"
#include "texture/texture.h"

#include "model.h"

namespace LearnVulkan
{
    class Application
    {
    public:
        Application()  = default;
        ~Application() = default;

        void run();

    private:
        void initWindow();
        void initVulkan();
        void createPipeline();
        void createRenderPass();
        void createCommandBuffers();
        void createSyncObjects();
        void mainLoop();
        void render();
        void recreateSwapChain();
        void cleanupSwapChain();
        void cleanUp();

    private:
        unsigned int mWidth{ 1280 };
        unsigned int mHeight{ 720 };

    private:
        int                         mCurrentFrame{ 0 };
        Wrapper::Window::Ptr        mWindow{ nullptr };
        Wrapper::Instance::Ptr      mInstance{ nullptr };
        Wrapper::Device::Ptr        mDevice{ nullptr };
        Wrapper::WindowSurface::Ptr mSurface{ nullptr };
        Wrapper::SwapChain::Ptr     mSwapChain{ nullptr };
        Wrapper::Pipeline::Ptr      mPipeline{ nullptr };
        Wrapper::RenderPass::Ptr    mRenderPass{ nullptr };

        Wrapper::CommandPool::Ptr                mCommandPool{ nullptr };
        std::vector<Wrapper::CommandBuffer::Ptr> mCommandBuffers{};

        std::vector<Wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};
        std::vector<Wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};
        std::vector<Wrapper::Fence::Ptr>     mFences{};

        UniformManager::Ptr mUniformManager{ nullptr };
        Model::Ptr          mModel{ nullptr };
        VPMatrices          mVPMatrices;
    };
}
