#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "utils/Buffer.h"
#include "utils/Texture.h"
#include "utils/UniformBufferObject.h"

struct Object {
	void allocateDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info);
	void bindDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);
	void writeDescriptorSets(VkDevice& device, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info);
	void createUniformBuffer(VkDevice device, VkPhysicalDevice physical_device);
	void updateUniformBuffer();

	void deleteUniformBuffer(VkDevice device);
	// private:
	// todo: プライベートにするために設計のし直し
	VkDescriptorSet descriptor_set;
	VkBuffer uniform_buffer;
	VkDeviceMemory device_memory;
};

