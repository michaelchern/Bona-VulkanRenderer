#pragma once

#include "base.h"
#include "vulkanWrapper/instance.h"
#include "vulkanWrapper/device.h"

namespace FF
{
	const int WIDTH = 800;
	const int HEIGHT = 600;

	class Application
	{
	public:
		Application() = default;

		~Application() = default;

		void run();

	private:
		void initWindow();
		void initVulkan();
		void mainLoop();
		void cleanUp();

		GLFWwindow* mWindow{ NULL };
		Wrapper::Instance::Ptr mInstance{ nullptr };
		Wrapper::Device::Ptr mDevice{ nullptr };
	};
}