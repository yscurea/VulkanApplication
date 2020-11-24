#include "Buffer.h"

VkDescriptorBufferInfo* Buffer::getDescriptorBufferInfo() {
	return &this->descriptor_buffer_info;
}