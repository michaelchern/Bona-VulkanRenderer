#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};