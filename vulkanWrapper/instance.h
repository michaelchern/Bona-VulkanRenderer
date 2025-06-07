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

		[[nodiscard]] VkInstance getInstance() const { return mInstance; }

	private:
		VkInstance mInstance{ VK_NULL_HANDLE };
		bool mEnableValidationLayer{ false };

		VkDebugUtilsMessengerEXT mDebugger{ VK_NULL_HANDLE };
	};
}