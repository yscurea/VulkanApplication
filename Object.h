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
	VkDescriptorSet graphics_descriptor_set;
	VkDescriptorSet offscreen_descriptor_set;
	VkBuffer uniform_buffer;
	VkDeviceMemory device_memory;
	// glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	void allocateGraphicsDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info);
	void allocateOffscreenDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info);

	void bindGraphicsDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);
	void bindOffscreenDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);

	void writeGraphicsDescriptorSets(VkDevice& device, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info);
	void writeOffscreenDescriptorSets(VkDevice& device, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info);

	void createUniformBuffer(VkDevice device, VkPhysicalDevice physical_device);
	void updateUniformBuffer(VkDevice device, Camera camera, VkExtent2D swapchain_extent);

	void deleteUniformBuffer(VkDevice device);
};

