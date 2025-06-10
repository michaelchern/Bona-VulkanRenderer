
#include "windowSurface.h"
#include <stdexcept>

namespace FF::Wrapper
{
	WindowSurface::WindowSurface(Instance::Ptr instance, Window::Ptr window)
	{
		if (glfwCreateWindowSurface(instance->getInstance(), window->getWindow(),nullptr,&mSurface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	WindowSurface::~WindowSurface()
	{
		
			vkDestroySurfaceKHR(mInstance->getInstance(), mSurface, nullptr);
		

		mInstance.reset();
	}
}