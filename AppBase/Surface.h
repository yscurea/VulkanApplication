#pragma once

#include <vulkan/vulkan.h>

// instance‚Æwindow‚ª•K—v

class Surface {
	VkSurfaceKHR surface;
public:
	void createSurface();
	void deleteSurface();
};