#pragma once

#include "base.h"
#include "device.h"
#include "window.h"
#include "windowSurface.h"
#include "renderPass.h"

namespace LearnVulkan::Wrapper
{
    // ������֧����Ϣ�ṹ��
    struct SwapChainSupportInfo
    {
        VkSurfaceCapabilitiesKHR mCapabilities;       // ������������С/���ͼ��������ͼ��ߴ緶Χ�ȣ�
        std::vector<VkSurfaceFormatKHR> mFormats;     // ֧�ֵ����ظ�ʽ�б�
        std::vector<VkPresentModeKHR> mPresentModes;  // ֧�ֵĳ���ģʽ�б�
    };

    // Vulkan ������������
    class SwapChain
    {
    public:
        using Ptr = std::shared_ptr<SwapChain>;
        static Ptr create(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface)
        {
            return std::make_shared<SwapChain>(device, window, surface);
        }

        // ���캯���������������������Դ
        SwapChain(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface);

        // �������������ٽ�������Դ
        ~SwapChain();

        // ��ѯ��ǰ�����豸�Ľ�����֧����Ϣ
        SwapChainSupportInfo querySwapChainSupportInfo();

        // �ӿ��ø�ʽ��ѡ����ѱ����ʽ
        VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        // �ӿ���ģʽ��ѡ����ѳ���ģʽ
        VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresenstModes);

        // ѡ�񽻻���ͼ��ߴ磨�ֱ��ʣ�
        VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        // Ϊ����������֡��������������Ⱦͨ����
        void createFrameBuffers(const RenderPass::Ptr& renderPass);

    public:
        // === ���ʷ��� ===

        [[nodiscard]] auto getFormat() const { return mSwapChainFormat; }                                   // ��ȡ������ͼ���ʽ

        [[nodiscard]] auto getImageCount() const { return mImageCount; }                                    // ��ȡ������ͼ������

        [[nodiscard]] auto getSwapChain() const { return mSwapChain; }                                      // ��ȡ�ײ�VkSwapchainKHR���

        [[nodiscard]] auto getFrameBuffer(const int index) const { return mSwapChainFrameBuffers[index]; }  // ��ȡָ��֡������

        [[nodiscard]] auto getExtent() const { return mSwapChainExtent; }                                   // ��ȡ������ͼ��ߴ�

    private:
        // ����ͼ����ͼ�����ڷ���ͼ��
        VkImageView createImageView(VkImage image,
                                    VkFormat format,
                                    VkImageAspectFlags aspectFlags,
                                    uint32_t mipLevels = 1);

    private:
        
        VkSwapchainKHR mSwapChain{ VK_NULL_HANDLE };          // Vulkan������������

        VkFormat  mSwapChainFormat;                           // ������ͼ���ʽ����RGBA8��
        VkExtent2D mSwapChainExtent;                          // ������ͼ��ߴ磨���x�߶ȣ�
        uint32_t mImageCount{ 0 };                            // �������е�ͼ������

        // �ɽ����������ͼ�񣨲���Ҫ�ֶ����٣�
        std::vector<VkImage> mSwapChainImages{};

        // ͼ����ͼ���ṩ�Խ�����ͼ��ķ��ʽӿ�
        std::vector<VkImageView> mSwapChainImageViews{};

        // ֡��������������Ⱦͨ����ͼ����ͼ
        std::vector<VkFramebuffer> mSwapChainFrameBuffers{};

        // ������Դ
        Device::Ptr mDevice{ nullptr };                       // �߼��豸����
        Window::Ptr mWindow{ nullptr };                       // �������ã����ڻ�ȡ�ߴ磩
        WindowSurface::Ptr mSurface{ nullptr };               // ���ڱ�������
    };
}