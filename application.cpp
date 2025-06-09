
#include <iostream>

#include "application.h"

namespace FF
{
	void Application::run()
	{
		initWindow();
		
		initVulkan();

		mainLoop();

		cleanUp();
	}

	void Application::initWindow()
	{
		glfwInit();

		// 设置环境，关掉 OpenGL API 并禁止窗口改变大小
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Window", nullptr, nullptr);

		if (!mWindow)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}
	}

	void Application::initVulkan()
	{
		mInstance = Wrapper::Instance::create(true);
		mDevice = Wrapper::Device::create(mInstance);
	}

	void Application::mainLoop()
	{
		while (!glfwWindowShouldClose(mWindow))
		{
			glfwPollEvents();
		}
	}

	void Application::cleanUp()
	{
		mDevice.reset();
		mInstance.reset();

		if (mWindow)
		{
			glfwDestroyWindow(mWindow);
			glfwTerminate();
		}
	}
}