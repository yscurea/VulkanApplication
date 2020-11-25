#pragma once

#include <vulkan/vulkan.h>

class Buffer {
public:
private:
	VkBuffer buffer;
	VkDescriptorBufferInfo descriptor_buffer_info;
};