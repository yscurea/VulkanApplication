#pragma once

#include <vulkan/vulkan.h>

// instance��window���K�v

class Surface {
	VkSurfaceKHR surface;
public:
	void createSurface();
	void deleteSurface();
};