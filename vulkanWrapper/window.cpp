#include "window.h"

namespace LearnVulkan::Wrapper
{

    static void windowResized(GLFWwindow* window, int width, int height)
    {
        auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        pUserData->mWindowResized = true;
    }

    Window::Window(const int& width, const int& height)
    {
        mWidth = width;
        mHeight = height;

        glfwInit();

        //设置环境，关掉opengl API
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        mWindow = glfwCreateWindow(mWidth, mHeight, "vulkan window", nullptr, nullptr);
        if (!mWindow)
        {
            std::cerr << "Error: failed to create window" << std::endl;
        }

        glfwSetWindowUserPointer(mWindow, this);
        glfwSetFramebufferSizeCallback(mWindow, windowResized);
    }

    Window::~Window()
    {
        glfwDestroyWindow(mWindow);
        glfwTerminate();
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