#pragma once

#include "instance.h"

namespace FF::Wrapper
{
	class Device
	{
	public:
		using Ptr = std::shared_ptr<Device>;
		static Ptr create(Instance::Ptr instance) { return std::make_shared<Device>(instance); }

		Device(Instance::Ptr instance);

		~Device();

		void pickPhysicalDevice();

		int rateDevice(VkPhysicalDevice device);

		bool isDeviceSuitable(VkPhysicalDevice device);

	private:
		VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };
		Instance::Ptr mInstance{ nullptr };
	};
}