#pragma once

#include <optional>

#include "instance.h"
#include "windowSurface.h"

namespace FF::Wrapper
{
	class Device
	{
	public:
		using Ptr = std::shared_ptr<Device>;
		static Ptr create(Instance::Ptr instance, WindowSurface::Ptr surface) { return std::make_shared<Device>(instance, surface); }

		Device(Instance::Ptr instance, WindowSurface::Ptr surface);

		~Device();

		void pickPhysicalDevice();

		int rateDevice(VkPhysicalDevice device);

		bool isDeviceSuitable(VkPhysicalDevice device);

		void initQueueFamilies(VkPhysicalDevice device);

		void createLogicalDevice();

		bool isQueueFamilyComplete();

	private:
		VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };
		Instance::Ptr mInstance{ nullptr };
		WindowSurface::Ptr mSurface{ nullptr };

		std::optional<uint32_t> mGraphicsQueueFamily;
		VkQueue mGraphicsQueue{ VK_NULL_HANDLE };

		std::optional<uint32_t> mPresentQueueFamily;
		VkQueue mPresentQueue{ VK_NULL_HANDLE };

		VkDevice mDevice{ VK_NULL_HANDLE };
	};
}