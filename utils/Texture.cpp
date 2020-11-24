#include "Texture.h"


VkDescriptorImageInfo* Texture::getDescriptorImageInfo() {
	return &this->descriptor_image_info;
}
