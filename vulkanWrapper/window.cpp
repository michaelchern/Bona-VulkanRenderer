

#include <stdexcept>

#include "window.h"


namespace FF::Wrapper
{

	Window::Window(const int& width, const int& height)
	{
		mWidth = width;
		mHeight = height;

		glfwInit();

		// 设置环境，关掉 OpenGL API 并禁止窗口改变大小
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		mWindow = glfwCreateWindow(mWidth, mHeight, "Vulkan Window", nullptr, nullptr);

		if (!mWindow)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}
	}

	Window::~Window()
	{
		if (mWindow)
		{
			glfwDestroyWindow(mWindow);
			glfwTerminate();
		}
	}

	bool Window::shouldClose()
	{
		return glfwWindowShouldClose(mWindow);
	}

	void Window::pollEvents()
	{
		glfwPollEvents();
	}
}