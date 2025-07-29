#pragma once

#include "base.h"
#include "instance.h"
#include "window.h"

namespace LearnVulkan::Wrapper
{
    class WindowSurface
    {
    public:
        using Ptr = std::shared_ptr<WindowSurface>;
        static Ptr create(Instance::Ptr& instance, Window::Ptr& window)
        {
            return std::make_shared<WindowSurface>(instance, window);
        }

        WindowSurface(Instance::Ptr& instance, Window::Ptr& window);

        ~WindowSurface();

        [[nodiscard]] auto getSurface() const { return mSurface; }

    private:
        VkSurfaceKHR  mSurface{ VK_NULL_HANDLE };
        Instance::Ptr mInstance{ nullptr };
    };
}
