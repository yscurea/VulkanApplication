#pragma once

#include <vulkan/vulkan.h>

class Texture {
public:
	VkDescriptorImageInfo* getDescriptorImageInfo();
private:
	VkDescriptorImageInfo descriptor_image_info;
};