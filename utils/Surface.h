#pragma once

#include <vulkan/vulkan.h>

// instance��window���K�v

class Surface {
	VkSurfaceKHR surface;
	void createSurface();
	void deleteSurface();
};