#pragma once

#include <memory>

#include "../base.h"

namespace FF::Wrapper
{
	class Window
	{
	public:
		using Ptr = std::shared_ptr<Window>;
		static Ptr create(return std::make_shared<Window>());

		Window();

		~Window();
			
	private:

	};
}