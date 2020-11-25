#pragma once

#include <vulkan/vulkan.h>

// instance‚Æwindow‚ª•K—v

class Surface {
	VkSurfaceKHR surface;
	void createSurface();
	void deleteSurface();
};