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

		// ���û������ص�OpenGL API ����ֹ���ڸı��С
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
		// ��ʼ��Vulkan��ص�����
		// ����������Vulkanʵ�����豸���������ȵĳ�ʼ������
	}

	void Application::mainLoop()
	{
		while (!glfwWindowShouldClose(m_Window))
		{
			glfwPollEvents();
			// ��Ⱦ����������
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