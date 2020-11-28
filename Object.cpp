#include "Object.h"

#include <stdexcept>
#include <array>

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



void Object::bindGraphicsDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) {
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &this->graphics_descriptor_set, 0, nullptr);
}
void Object::bindOffscreenDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) {
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &this->offscreen_descriptor_set, 0, nullptr);
}
void Object::writeGraphicsDescriptorSets(VkDevice& device, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info) {
	std::array<VkWriteDescriptorSet, 2> write_descriptor_sets{};
	// Binding 0: ユニフォームバッファ
	write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_sets[0].dstSet = this->graphics_descriptor_set;
	write_descriptor_sets[0].dstBinding = 0;
	write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_descriptor_sets[0].pBufferInfo = buffer_info;
	write_descriptor_sets[0].descriptorCount = 1;

	// Binding 1: テクスチャサンプラー
	write_descriptor_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_sets[1].dstSet = this->graphics_descriptor_set;
	write_descriptor_sets[1].dstBinding = 1;
	write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_descriptor_sets[1].pImageInfo = image_info;
	write_descriptor_sets[1].descriptorCount = 1;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);

}
void Object::writeOffscreenDescriptorSets(VkDevice& device, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info) {
	std::array<VkWriteDescriptorSet, 2> write_descriptor_sets{};
	// Binding 0: ユニフォームバッファ
	write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_sets[0].dstSet = this->offscreen_descriptor_set;
	write_descriptor_sets[0].dstBinding = 0;
	write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_descriptor_sets[0].pBufferInfo = buffer_info;
	write_descriptor_sets[0].descriptorCount = 1;

	// Binding 1: テクスチャサンプラー
	write_descriptor_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_sets[1].dstSet = this->offscreen_descriptor_set;
	write_descriptor_sets[1].dstBinding = 1;
	write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_descriptor_sets[1].pImageInfo = image_info;
	write_descriptor_sets[1].descriptorCount = 1;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);

}
void Object::allocateGraphicsDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info) {
	if (vkAllocateDescriptorSets(device, &allocate_info, &this->graphics_descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets");
	}
}
void Object::allocateOffscreenDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info) {
	if (vkAllocateDescriptorSets(device, &allocate_info, &this->offscreen_descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets");
	}
}

void Object::createUniformBuffer(VkDevice device, VkPhysicalDevice physical_device) {
	std::random_device seed_gen;
	this->position.x = (float)(seed_gen() % 30);
	this->position.y = (float)(seed_gen() % 30);
	this->position.z = (float)(seed_gen() % 30);
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
void Object::createUniformBufferOffscreen(VkDevice device, VkPhysicalDevice physical_device) {
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = sizeof(OffscreenUniformBufferObject);
	buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &buffer_info, nullptr, &this->offscreen_uniform_buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, this->offscreen_uniform_buffer, &memRequirements);

	auto properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = memRequirements.size;
	alloc_info.memoryTypeIndex = findMemoryType(physical_device, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &alloc_info, nullptr, &this->offscreen_device_memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate memory,");
	}

	vkBindBufferMemory(device, this->offscreen_uniform_buffer, this->offscreen_device_memory, 0);
}
void Object::updateUniformBuffer(VkDevice device, Camera camera, VkExtent2D swapchain_extent) {
	UniformBufferObject ubo{};

	auto translate = glm::translate(glm::mat4(1.0f), this->position);
	auto rotate = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 1.0f));
	ubo.model = translate * rotate;
	ubo.view = glm::lookAt(camera.position, camera.target_position, glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.proj = glm::perspective(camera.fov, swapchain_extent.width / (float)swapchain_extent.height, camera.near_clip, camera.far_clip);

	void* data;
	vkMapMemory(device, this->device_memory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, this->device_memory);
}
void Object::updateUniformBufferOffscreen(VkDevice device, Light light, VkExtent2D swapchain_extent) {
	OffscreenUniformBufferObject oubo{};

	oubo.model = glm::mat4(1.0f);
	oubo.view = glm::lookAt(light.position, glm::vec3(0.0f), glm::vec3(0, 1, 0));
	oubo.projection = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);

	void* data;
	vkMapMemory(device, this->offscreen_device_memory, 0, sizeof(oubo), 0, &data);
	memcpy(data, &oubo, sizeof(oubo));
	vkUnmapMemory(device, this->offscreen_device_memory);
}
void Object::deleteUniformBuffer(VkDevice device) {
	vkDestroyBuffer(device, this->uniform_buffer, nullptr);
	vkFreeMemory(device, this->device_memory, nullptr);
}
