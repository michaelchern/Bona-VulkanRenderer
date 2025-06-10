#pragma once

#include <memory>

#include "../base.h"

namespace FF::Wrapper
{
	class Window
	{
	public:
		using Ptr = std::shared_ptr<Window>;
		static Ptr create(const int& width, const int& height) { return std::make_shared<Window>(width, height); }

		Window(const int& width, const int& height);

		~Window();

		bool shouldClose();

		void pollEvents();

		[[nodiscard]] auto getWindow() const { return mWindow; }
			
	private:
		int mWidth{ 0 };
		int mHeight{ 0 };
		GLFWwindow* mWindow{ NULL };
	};
}