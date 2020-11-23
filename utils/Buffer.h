#pragma once

class Buffer {
public:
	VkDescriptorBufferInfo* getDescriptorBufferInfo();
private:
	VkDescriptorBufferInfo descriptor_buffer_info;
};