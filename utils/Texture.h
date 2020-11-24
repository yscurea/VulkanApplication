#pragma once

#include <vulkan/vulkan.h>

class Texture {
public:
	VkDescriptorImageInfo* getDescriptorImageInfo();
private:
	VkImage image;
	VkImageView image_view;
	VkDescriptorImageInfo descriptor_image_info;
};