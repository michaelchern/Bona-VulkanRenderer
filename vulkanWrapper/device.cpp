#include "device.h"

namespace LearnVulkan::Wrapper
{

	Device::Device(Instance::Ptr instance, WindowSurface::Ptr surface)
	{
		mInstance = instance;
		mSurface = surface;
		pickPhysicalDevice();
		initQueueFamilies(mPhysicalDevice);
		createLogicalDevice();
	}

	Device::~Device()
	{
		vkDestroyDevice(mDevice, nullptr);
		mSurface.reset();
		mInstance.reset();
	}

	// ѡ����������豸
	void Device::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;

		// 1. ��ȡ���������豸����
		vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("Error:failed to enumeratePhysicalDevice");
		}

		// 2. ��ȡ���������豸���
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, devices.data());

		// 3. ʹ��multimap�Զ������������豸
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : devices)
		{
			// ���豸����
			int score = rateDevice(device);
			candidates.insert(std::make_pair(score, device));
		}

		// 4. ѡ����߷��Һ��ʵ��豸
		if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second))
		{
			mPhysicalDevice = candidates.rbegin()->second;
		}

		if (mPhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Error:failed to get physical device");
		}
	}

	// �豸���ֺ���
	int Device::rateDevice(VkPhysicalDevice device)
	{
		int score = 0;

		// 1. ��ȡ�豸���ԣ����ơ����͡��汾�ȣ�
		VkPhysicalDeviceProperties  deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		// 2. ��ȡ�豸���ԣ�֧�ֵĹ��ܣ�
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// 3. �����Կ��ӷ֣����ܸ��ã�
		if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		// 4. ֧�ָ�������ӷ�
		score += deviceProp.limits.maxImageDimension2D;

		// 5. �����м�����ɫ��֧��
		if (!deviceFeatures.geometryShader)
		{
			return 0;
		}

		return score;
	}

	// ����豸�Ƿ��������Ҫ��
	bool Device::isDeviceSuitable(VkPhysicalDevice device)
	{
		// 1. ��ȡ�豸����
		VkPhysicalDeviceProperties  deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		// 2. ��ȡ�豸����
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// 3. ���������������
		return deviceProp.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&  // �����Կ�
			deviceFeatures.geometryShader &&         // ֧�ּ�����ɫ��
			deviceFeatures.samplerAnisotropy;        // ֧�ָ������Թ���
	}

	// ��ʼ������������
	void Device::initQueueFamilies(VkPhysicalDevice device)
	{
		uint32_t queueFamilyCount = 0;

		// 1. ��ȡ����������
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		// 2. ��ȡ����������
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			// 3. ����֧��ͼ�β����Ķ�����
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				mGraphicQueueFamily = i;
			}

			// 4. ����֧�ֱ�����ֵĶ�����
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface->getSurface(), &presentSupport);

			if (presentSupport)
			{
				mPresentQueueFamily = i;
			}

			// 5. ����ҵ����ֶ��У���ǰ����
			if (isQueueFamilyComplete()) {
				break;
			}

			++i;
		}
	}

	// �����߼��豸
	void Device::createLogicalDevice()
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		// 1. �ռ���Ҫ�����Ķ����壨ȥ�أ�
		std::set<uint32_t> queueFamilies = { mGraphicQueueFamily.value(), mPresentQueueFamily.value() };

		float queuePriority = 1.0;  // �������ȼ�

		// 2. Ϊÿ�������崴��������Ϣ
		for (uint32_t queueFamily : queueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		// 3. �����豸���ԣ��������Թ��ˣ�
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		// 4. ��д�߼��豸������Ϣ
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		// 5. �����豸��չ
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceRequiredExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceRequiredExtensions.data();

		// 6. ������֤�㣨���ʵ�������ˣ�
		if (mInstance->getEnableValidationLayer())
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
		}

		// 7. �����߼��豸
		if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to create logical device");
		}

		// 8. ��ȡ���о��
		vkGetDeviceQueue(mDevice, mGraphicQueueFamily.value(), 0, &mGraphicQueue);
		vkGetDeviceQueue(mDevice, mPresentQueueFamily.value(), 0, &mPresentQueue);
	}

	// ���������Ƿ�����
	bool Device::isQueueFamilyComplete()
	{
		return mGraphicQueueFamily.has_value() && mPresentQueueFamily.has_value();
	}
}