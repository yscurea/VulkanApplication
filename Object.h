#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <random>


#include "Camera.h"
#include "Light.h"
#include "utils/Buffer.h"
#include "utils/Texture.h"
#include "utils/UniformBufferObject.h"
#include "OffscreenUniformBufferObject.h"

class Object {
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	OffscreenUniformBufferObject oubo{};
public:
	VkDescriptorSet graphics_descriptor_set;
	VkDescriptorSet offscreen_descriptor_set;
	VkBuffer uniform_buffer;
	VkBuffer offscreen_uniform_buffer;
	VkDeviceMemory device_memory;
	VkDeviceMemory offscreen_device_memory;
	void allocateGraphicsDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info);
	void allocateOffscreenDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info);

	void bindGraphicsDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);
	void bindOffscreenDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);

	void writeDescriptorSets();
	void writeGraphicsDescriptorSets(VkDevice& device, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info);
	void writeOffscreenDescriptorSets(VkDevice& device, VkDescriptorBufferInfo* buffer_info);

	void createUniformBuffer(VkDevice device, VkPhysicalDevice physical_device);
	void createUniformBufferOffscreen(VkDevice device, VkPhysicalDevice physical_device);
	void updateUniformBuffer(VkDevice device, Light light, Camera camera, VkExtent2D swapchain_extent);
	void updateUniformBufferOffscreen(VkDevice device, Light light, VkExtent2D swapchain_extent);

	void deleteUniformBuffer(VkDevice& device);
};

