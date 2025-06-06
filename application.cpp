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

		// 设置环境，关掉OpenGL API 并禁止窗口改变大小
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Window", nullptr, nullptr);

		if (!m_Window)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}
	}

	void Application::initVulkan()
	{
		// 初始化Vulkan相关的内容
		// 这里可以添加Vulkan实例、设备、交换链等的初始化代码
	}

	void Application::mainLoop()
	{
		while (!glfwWindowShouldClose(m_Window))
		{
			glfwPollEvents();
			// 渲染或其他操作
		}
	}

	void Application::cleanUp()
	{
		if (m_Window)
		{
			glfwDestroyWindow(m_Window);
			glfwTerminate();
		}
	}
}