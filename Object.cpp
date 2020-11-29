#include "Object.h"

#include <stdexcept>
#include <array>
#include <iostream>

static uint32_t findMemoryType(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");
}



void Object::bindDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) {
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &this->descriptor_set, 0, nullptr);
}
void Object::writeDescriptorSets(VkDevice& device, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info) {
	std::array<VkWriteDescriptorSet, 2> write_descriptor_sets{};
	// Binding 0: ユニフォームバッファ
	write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_sets[0].dstSet = this->descriptor_set;
	write_descriptor_sets[0].dstBinding = 0;
	write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_descriptor_sets[0].pBufferInfo = buffer_info;
	write_descriptor_sets[0].descriptorCount = 1;

	// Binding 1: テクスチャサンプラー
	write_descriptor_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_sets[1].dstSet = this->descriptor_set;
	write_descriptor_sets[1].dstBinding = 1;
	write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_descriptor_sets[1].pImageInfo = image_info;
	write_descriptor_sets[1].descriptorCount = 1;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);

}
void Object::allocateDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info) {
	if (vkAllocateDescriptorSets(device, &allocate_info, &this->descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets");
	}
}

void Object::createUniformBuffer(VkDevice device, VkPhysicalDevice physical_device) {
	std::random_device seed_gen;
	this->position.x = (float)(seed_gen() % 30);
	this->position.y = (float)(seed_gen() % 30);
	this->position.z = (float)(seed_gen() % 30);

	this->velocity.x = (float)(seed_gen() % 100 / 100.0);
	this->velocity.y = (float)(seed_gen() % 100 / 100.0);
	this->velocity.z = (float)(seed_gen() % 100 / 100.0);

	this->rotation.x = (float)(seed_gen() % 30);
	this->rotation.y = (float)(seed_gen() % 30);
	this->rotation.z = (float)(seed_gen() % 30);

	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = sizeof(UniformBufferObject);
	buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &buffer_info, nullptr, &this->uniform_buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, this->uniform_buffer, &memRequirements);

	auto properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = memRequirements.size;
	alloc_info.memoryTypeIndex = findMemoryType(physical_device, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &alloc_info, nullptr, &this->device_memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to aVkDevice device, VkExtent2D swapchain_extentllocate buffer memory!");
	}

	vkBindBufferMemory(device, this->uniform_buffer, this->device_memory, 0);
}
void Object::updateUniformBuffer(VkDevice device, float time, Camera camera, VkExtent2D swapchain_extent) {
	this->position.x += this->velocity.x * 0.01f;
	this->position.y += this->velocity.y * 0.01f;
	this->position.z += this->velocity.z * 0.01f;
	this->rotation.x += this->velocity.x * 0.001f;
	this->rotation.y += this->velocity.y * 0.001f;
	this->rotation.z += this->velocity.z * 0.001f;


	UniformBufferObject ubo{};

	ubo.model = glm::translate(glm::mat4(1.0f), this->position);
	ubo.model = glm::rotate(ubo.model, glm::degrees(this->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	ubo.model = glm::rotate(ubo.model, glm::degrees(this->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.model = glm::rotate(ubo.model, glm::degrees(this->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(camera.position, camera.target_position, glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.proj = glm::perspective(camera.fov, swapchain_extent.width / (float)swapchain_extent.height, camera.near_clip, camera.far_clip);

	void* data;
	vkMapMemory(device, this->device_memory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, this->device_memory);
}
void Object::deleteUniformBuffer(VkDevice device) {
	vkDestroyBuffer(device, this->uniform_buffer, nullptr);
	vkFreeMemory(device, this->device_memory, nullptr);
}
