#pragma once

#include "base.h"

namespace LearnVulkan::Wrapper
{
	// ��ʾһ��Vulkanʵ���ķ�װ��
	class Instance
	{
	public:
		// ����ָ��������򻯹���ָ���ʹ��
		using Ptr = std::shared_ptr<Instance>;

		// ��̬��������������Instance����Ĺ���ָ��
		// @param enableValidationLayer �Ƿ�����Vulkan��֤�㣨���ڵ��ԣ�
		static Ptr create(bool enableValidationLayer) { return std::make_shared<Instance>(enableValidationLayer); }

		// ���캯��������Vulkanʵ��
		// @param enableValidationLayer ������֤�㿪��
		Instance(bool enableValidationLayer);

		// ��������������Vulkanʵ���͵��Թ���
		~Instance();

		// ��ӡ���п��õ�Vulkan��չ�б������ã�
		void printAvailableExtensions();

		// ��ȡ�����Vulkan��չ�б�
		// @return ����const char*���飬����GLFW�ȱ�Ҫ��չ��
		std::vector<const char*> getRequiredExtensions();

		// ����������֤���Ƿ����
		// @return ��֤��֧��״̬
		bool checkValidationLayerSupport();

		// ����Vulkan���Իص�������������֤��ʱ��Ч��
		void setupDebugger();

		// ��ȡ�ײ�VkInstance����
		// [[nodiscard]] ��������ߺ��Է���ֵ
		[[nodiscard]] VkInstance getInstance() const { return mInstance; }

		// ��ȡ��֤������״̬
		[[nodiscard]] bool getEnableValidationLayer() const { return mEnableValidationLayer; }

	private:
		VkInstance mInstance{ VK_NULL_HANDLE };                // Vulkanʵ�����
		bool mEnableValidationLayer{ false };                  // ��֤�����ñ�־
		VkDebugUtilsMessengerEXT mDebugger{ VK_NULL_HANDLE };  // Vulkan������ʹ���
	};
}