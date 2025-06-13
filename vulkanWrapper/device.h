#pragma once

#include "base.h"
#include "instance.h"
#include "windowSurface.h"

namespace LearnVulkan::Wrapper
{
	// ������豸��չ�б� (������ + ά����չ)
	const std::vector<const char*> deviceRequiredExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,    // ֧�ֽ��������ܣ�֡�������
		VK_KHR_MAINTENANCE1_EXTENSION_NAME  // �ṩAPI�Ľ��ͼ�����֧��
	};

	// Vulkan �豸�����ࣨ��װ�����豸���߼��豸��
	class Device
	{
	public:
		using Ptr = std::shared_ptr<Device>;

		static Ptr create(Instance::Ptr instance, WindowSurface::Ptr surface)
		{
			return std::make_shared<Device>(instance, surface);
		}

		// ���캯������ʼ���豸ѡ�����
		Device(Instance::Ptr instance, WindowSurface::Ptr surface);

		// ���������������߼��豸
		~Device();

		// ѡ����õ������豸��GPU��
		void pickPhysicalDevice();

		// �������豸���֣�����ѡ�����GPU��
		int rateDevice(VkPhysicalDevice device);

		// ����豸�Ƿ��������Ҫ��
		bool isDeviceSuitable(VkPhysicalDevice device);

		// ��ʼ���豸�Ķ����壨ͼ�ζ��� + ���ֶ��У�
		void initQueueFamilies(VkPhysicalDevice device);

		// �����߼��豸��VkDevice��
		void createLogicalDevice();

		// ���������Ƿ�������ͼ��+���֣�
		bool isQueueFamilyComplete();

		// === ���ʷ��� ===
		[[nodiscard]] auto getDevice() const { return mDevice; }                          // ��ȡ�߼��豸
		[[nodiscard]] auto getPhysicalDevice() const { return mPhysicalDevice; }          // ��ȡ�����豸

		[[nodiscard]] auto getGraphicQueueFamily() const { return mGraphicQueueFamily; }  // ͼ�ζ���������
		[[nodiscard]] auto getPresentQueueFamily() const { return mPresentQueueFamily; }  // ���ֶ���������

		[[nodiscard]] auto getGraphicQueue() const { return mGraphicQueue; }              // ͼ�ζ��о��
		[[nodiscard]] auto getPresentQueue() const { return mPresentQueue; }              // ���ֶ��о��
	private:
		VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };  // �����豸��� (GPU)
		Instance::Ptr mInstance{ nullptr };                  // Vulkan ʵ������
		WindowSurface::Ptr mSurface{ nullptr };              // ���ڱ�������

		// ���������
		std::optional<uint32_t> mGraphicQueueFamily;         // ͼ�ζ���������������δ���ã�
		VkQueue	mGraphicQueue{ VK_NULL_HANDLE };             // ͼ�ζ��о��

		std::optional<uint32_t> mPresentQueueFamily;         // ���ֶ���������������δ���ã�
		VkQueue mPresentQueue{ VK_NULL_HANDLE };             // ���ֶ��о��

		//�߼��豸
		VkDevice mDevice{ VK_NULL_HANDLE };                  // �߼��豸���
	};
}