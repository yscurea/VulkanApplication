#include "Object.h"

#include <stdexcept>

void Object::draw(VkCommandBuffer command_buffer) {
	this->model->drawIndex(command_buffer);
}
void Object::setModel(Model* model) {
	this->model = model;
}
void Object::loadModel(std::string model_path) {
	this->model->load(model_path);
}
void Object::bindDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) {
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &this->descriptor_set, 0, nullptr);
}
void Object::allocateDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info) {
	if (vkAllocateDescriptorSets(device, &allocate_info, &this->descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets");
	}
}
