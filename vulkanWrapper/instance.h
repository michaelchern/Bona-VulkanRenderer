#pragma once

#include "../base.h"
#include <vector>

namespace FF::Wrapper
{
	class Instance
	{
	public:
		using Ptr = std::shared_ptr<Instance>;
		static Ptr create(bool enableValidationLayer) { return std::make_shared<Instance>(enableValidationLayer); }

		Instance(bool enableValidationLayer);
		~Instance();

		void printAvailableExtensions();
		std::vector<const char*> getRequiredExtensions();

		bool checkValidationLayerSupport();
		void setupDebugger();

	private:
		VkInstance mInstance;
		bool mEnableValidationLayer{ false };

		VkDebugUtilsMessengerEXT mDebugger;
	};
}