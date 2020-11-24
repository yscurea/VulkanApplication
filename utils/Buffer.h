#pragma once

#include <vulkan/vulkan.h>

class Buffer {
public:
	VkDescriptorBufferInfo* getDescriptorBufferInfo();
private:
	VkBuffer buffer;
	VkDescriptorBufferInfo descriptor_buffer_info;
};