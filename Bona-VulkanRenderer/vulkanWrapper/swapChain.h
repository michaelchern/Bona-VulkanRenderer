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

        // SwapChain �๹�캯�������𴴽� Vulkan ��������Swap Chain���������Դ
        // ����˵����
        // - device: Vulkan �߼��豸ָ�룬������ GPU ����
        // - window: ���ڶ���ָ�룬�ṩ����ϵͳ��ؽӿڣ��� GLFW ���ڣ�
        // - surface: Vulkan ���ڱ���ָ�룬���Ӵ���ϵͳ�� Vulkan �����������ڳ���ͼ����Ļ��
        // - commandPool: �����ָ�룬���ڷ��� Vulkan ����������˴��������ͼ�񲼾�ת����
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

        // ����������Ϊ Vulkan ͼ�񴴽�ͼ����ͼ��Image View��
        // ͼ����ͼ�� Vulkan �з���ͼ��ĺ��ķ�ʽ������ָ����ν���ͼ��ĸ�ʽ������Դ���� Mipmap �㼶������㣩��
        // ����˵����
        // - image: ��������ͼ�� Vulkan ͼ������罻�����е���ɫͼ������ͼ��
        // - format: ͼ������ظ�ʽ������ͼ����ĸ�ʽһ�£��� VK_FORMAT_B8G8R8A8_SRGB��
        // - aspectFlags: ͼ��ķ����־��ָ����ͼ�ķ������ͣ�����ɫͨ�������ͨ���ȣ�
        //   ����ֵ��VK_IMAGE_ASPECT_COLOR_BIT����ɫͼ�񣩡�VK_IMAGE_ASPECT_DEPTH_BIT�����ͼ��
        // - mipLevels: ͼ��� Mipmap �㼶������ͨ��Ϊ 1����ͼ�������� Mipmap ��Ϊʵ�ʲ㼶����
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
