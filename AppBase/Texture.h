#pragma once

#include <vulkan/vulkan.h>

class Texture {
	VkImage texture_image;
	VkDeviceMemory texture_image_memory;
	VkImageView texture_image_view;
	VkSampler texture_sampler;
	uint32_t mip_levels;
	VkDescriptorImageInfo descriptor_image_info;
};