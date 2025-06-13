#include "instance.h"

namespace LearnVulkan::Wrapper
{
	// 调试回调函数 - 处理 Vulkan 验证层输出的消息
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,    // 消息严重级别
		VkDebugUtilsMessageTypeFlagsEXT messageType,               // 消息类型
		const VkDebugUtilsMessengerCallbackDataEXT* pMessageData,  // 消息数据
		void* pUserData)                                           // 用户自定义数据
	{
		// 简单打印验证层消息到控制台
		std::cout << "ValidationLayer: " << pMessageData->pMessage << std::endl;

		// 返回 VK_FALSE 表示不中止 Vulkan 调用
		return VK_FALSE;
	}

	// 创建调试信使的扩展函数封装（因为这是扩展功能）
	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,  // 创建信息
		const VkAllocationCallbacks* pAllocator,                // 内存分配器
		VkDebugUtilsMessengerEXT* debugMessenger)               // 输出调试信使句柄
	{
		// 动态获取扩展函数指针
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, debugMessenger);
		}
		else
		{
			// 扩展不可用
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	// 销毁调试信使的扩展函数封装
	static void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT  debugMessenger,  // 要销毁的调试信使
		const VkAllocationCallbacks* pAllocator)   // 内存分配器
	{
		// 动态获取扩展函数指针
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			return func(instance, debugMessenger, pAllocator);
		}
	}

	// 构造函数：创建 Vulkan 实例
	Instance::Instance(bool enableValidationLayer)
	{
		mEnableValidationLayer = enableValidationLayer;

		// 检查验证层支持（如果启用）
		if (mEnableValidationLayer && !checkValidationLayerSupport())
		{
			throw std::runtime_error("Error: validation layer is not supported");
		}

		// 打印可用扩展（调试用）
		printAvailableExtensions();

		// 设置应用程序信息
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "vulkanLearning";            // 应用程序名称
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // 应用版本
		appInfo.pEngineName = "NO ENGINE";                      // 引擎名称
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // 引擎版本
		appInfo.apiVersion = VK_API_VERSION_1_0;                // Vulkan API 版本

		// 设置实例创建信息
		VkInstanceCreateInfo instCreateInfo = {};
		instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instCreateInfo.pApplicationInfo = &appInfo;

		// 获取并设置必需扩展
		auto extensions = getRequiredExtensions();
		instCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instCreateInfo.ppEnabledExtensionNames = extensions.data();

		// 设置验证层（如果启用）
		if (mEnableValidationLayer)
		{
			instCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			instCreateInfo.enabledLayerCount = 0;  // 禁用所有验证层
		}

		// 创建 Vulkan 实例
		if (vkCreateInstance(&instCreateInfo, nullptr, &mInstance) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to create instance");
		}

		// 设置调试回调（如果启用验证层）
		setupDebugger();
	}

	// 析构函数：清理 Vulkan 资源
	Instance::~Instance()
	{
		// 先销毁调试信使（如果存在）
		if (mEnableValidationLayer)
		{
			DestroyDebugUtilsMessengerEXT(mInstance, mDebugger, nullptr);
		}

		// 销毁 Vulkan 实例
		vkDestroyInstance(mInstance, nullptr);
	}

	// 打印所有可用的 Vulkan 扩展
	void Instance::printAvailableExtensions()
	{
		uint32_t extensionCount = 0;

		// 1. 获取扩展数量
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		// 2. 分配空间并获取扩展属性
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		// 3. 打印所有可用扩展
		std::cout << "Available extensions:" << std::endl;
		for (const auto& extension : extensions)
		{
			std::cout << extension.extensionName << std::endl;
		}
	}

	// 获取应用程序必需的扩展列表
	std::vector<const char*> Instance::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;

		// 1. 从 GLFW 获取窗口系统必需的扩展
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// 2. 复制 GLFW 返回的扩展
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// 3. 添加调试工具扩展（如果启用验证层）
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	// 检查系统是否支持请求的验证层
	bool Instance::checkValidationLayerSupport()
	{
		uint32_t layerCount = 0;

		// 1. 获取可用验证层数量
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// 2. 获取所有可用验证层
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// 3. 检查每个请求的验证层是否可用
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

	// 设置调试回调系统
	void Instance::setupDebugger()
	{
		// 如果未启用验证层则直接返回
		if (!mEnableValidationLayer) { return; }

		// 配置调试信使创建信息
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		// 设置要接收的消息严重级别
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |  // 详细诊断信息
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |  // 警告
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;     // 错误

		// 设置要接收的消息类型
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |      // 常规信息
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |   // 违反规范/最佳实践
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;   // 性能问题

		// 设置回调函数
		createInfo.pfnUserCallback = debugCallBack;

		// 没有用户数据传递
		createInfo.pUserData = nullptr;

		// 创建调试信使对象
		if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugger) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to create debugger");
		}
	}
}