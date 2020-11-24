#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <random>


#include "Camera.h"
#include "utils/Buffer.h"
#include "utils/Texture.h"
#include "utils/UniformBufferObject.h"

class Object {
public:
	VkDescriptorSet descriptor_set;
	VkBuffer uniform_buffer;
	VkDeviceMemory device_memory;
	// glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	void allocateDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info);
	void bindDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);
	void writeDescriptorSets(VkDevice& device, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info);
	void createUniformBuffer(VkDevice device, VkPhysicalDevice physical_device);
	void updateUniformBuffer(VkDevice device, Camera camera, VkExtent2D swapchain_extent);

	void deleteUniformBuffer(VkDevice device);
};

