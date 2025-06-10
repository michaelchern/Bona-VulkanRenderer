#pragma once

#include "base.h"
#include "vulkanWrapper/instance.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/window.h"
#include "vulkanWrapper/windowSurface.h"

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

		Wrapper::Window::Ptr mWindow{ nullptr };
		Wrapper::Instance::Ptr mInstance{ nullptr };
		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::WindowSurface::Ptr mSurface{ nullptr };
	};
}