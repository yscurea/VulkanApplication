#pragma once

#include <vulkan/vulkan.h>

class Surface {
	VkSurfaceKHR surface;
	void createSurface();
	void deleteSurface();
};