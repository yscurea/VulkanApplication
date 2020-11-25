#pragma once

#include <vulkan/vulkan.h>

class Buffer {
	VkBuffer buffer;
	VkDeviceMemory device_memory;
};