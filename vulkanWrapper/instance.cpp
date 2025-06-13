#include "instance.h"

namespace LearnVulkan::Wrapper
{
	// ���Իص����� - ���� Vulkan ��֤���������Ϣ
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,    // ��Ϣ���ؼ���
		VkDebugUtilsMessageTypeFlagsEXT messageType,               // ��Ϣ����
		const VkDebugUtilsMessengerCallbackDataEXT* pMessageData,  // ��Ϣ����
		void* pUserData)                                           // �û��Զ�������
	{
		// �򵥴�ӡ��֤����Ϣ������̨
		std::cout << "ValidationLayer: " << pMessageData->pMessage << std::endl;

		// ���� VK_FALSE ��ʾ����ֹ Vulkan ����
		return VK_FALSE;
	}

	// ����������ʹ����չ������װ����Ϊ������չ���ܣ�
	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,  // ������Ϣ
		const VkAllocationCallbacks* pAllocator,                // �ڴ������
		VkDebugUtilsMessengerEXT* debugMessenger)               // ���������ʹ���
	{
		// ��̬��ȡ��չ����ָ��
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, debugMessenger);
		}
		else
		{
			// ��չ������
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	// ���ٵ�����ʹ����չ������װ
	static void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT  debugMessenger,  // Ҫ���ٵĵ�����ʹ
		const VkAllocationCallbacks* pAllocator)   // �ڴ������
	{
		// ��̬��ȡ��չ����ָ��
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			return func(instance, debugMessenger, pAllocator);
		}
	}

	// ���캯�������� Vulkan ʵ��
	Instance::Instance(bool enableValidationLayer)
	{
		mEnableValidationLayer = enableValidationLayer;

		// �����֤��֧�֣�������ã�
		if (mEnableValidationLayer && !checkValidationLayerSupport())
		{
			throw std::runtime_error("Error: validation layer is not supported");
		}

		// ��ӡ������չ�������ã�
		printAvailableExtensions();

		// ����Ӧ�ó�����Ϣ
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "vulkanLearning";            // Ӧ�ó�������
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // Ӧ�ð汾
		appInfo.pEngineName = "NO ENGINE";                      // ��������
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // ����汾
		appInfo.apiVersion = VK_API_VERSION_1_0;                // Vulkan API �汾

		// ����ʵ��������Ϣ
		VkInstanceCreateInfo instCreateInfo = {};
		instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instCreateInfo.pApplicationInfo = &appInfo;

		// ��ȡ�����ñ�����չ
		auto extensions = getRequiredExtensions();
		instCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instCreateInfo.ppEnabledExtensionNames = extensions.data();

		// ������֤�㣨������ã�
		if (mEnableValidationLayer)
		{
			instCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			instCreateInfo.enabledLayerCount = 0;  // ����������֤��
		}

		// ���� Vulkan ʵ��
		if (vkCreateInstance(&instCreateInfo, nullptr, &mInstance) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to create instance");
		}

		// ���õ��Իص������������֤�㣩
		setupDebugger();
	}

	// �������������� Vulkan ��Դ
	Instance::~Instance()
	{
		// �����ٵ�����ʹ��������ڣ�
		if (mEnableValidationLayer)
		{
			DestroyDebugUtilsMessengerEXT(mInstance, mDebugger, nullptr);
		}

		// ���� Vulkan ʵ��
		vkDestroyInstance(mInstance, nullptr);
	}

	// ��ӡ���п��õ� Vulkan ��չ
	void Instance::printAvailableExtensions()
	{
		uint32_t extensionCount = 0;

		// 1. ��ȡ��չ����
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		// 2. ����ռ䲢��ȡ��չ����
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		// 3. ��ӡ���п�����չ
		std::cout << "Available extensions:" << std::endl;
		for (const auto& extension : extensions)
		{
			std::cout << extension.extensionName << std::endl;
		}
	}

	// ��ȡӦ�ó���������չ�б�
	std::vector<const char*> Instance::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;

		// 1. �� GLFW ��ȡ����ϵͳ�������չ
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// 2. ���� GLFW ���ص���չ
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// 3. ��ӵ��Թ�����չ�����������֤�㣩
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	// ���ϵͳ�Ƿ�֧���������֤��
	bool Instance::checkValidationLayerSupport()
	{
		uint32_t layerCount = 0;

		// 1. ��ȡ������֤������
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// 2. ��ȡ���п�����֤��
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// 3. ���ÿ���������֤���Ƿ����
		for (const auto& layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProp : availableLayers)
			{
				if (std::strcmp(layerName, layerProp.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	// ���õ��Իص�ϵͳ
	void Instance::setupDebugger()
	{
		// ���δ������֤����ֱ�ӷ���
		if (!mEnableValidationLayer) { return; }

		// ���õ�����ʹ������Ϣ
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		// ����Ҫ���յ���Ϣ���ؼ���
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |  // ��ϸ�����Ϣ
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |  // ����
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;     // ����

		// ����Ҫ���յ���Ϣ����
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |      // ������Ϣ
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |   // Υ���淶/���ʵ��
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;   // ��������

		// ���ûص�����
		createInfo.pfnUserCallback = debugCallBack;

		// û���û����ݴ���
		createInfo.pUserData = nullptr;

		// ����������ʹ����
		if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugger) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to create debugger");
		}
	}
}