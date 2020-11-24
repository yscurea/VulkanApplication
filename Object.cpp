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
void Object::writeDescriptorSets(VkDevice& device) {
	std::array<VkWriteDescriptorSet, 2> write_descriptor_sets{};
	// Binding 0: ユニフォームバッファ
	write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_sets[0].dstSet = this->descriptor_set;
	write_descriptor_sets[0].dstBinding = 0;
	write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_descriptor_sets[0].pBufferInfo = this->uniform_buffer.getDescriptorBufferInfo();
	write_descriptor_sets[0].descriptorCount = 1;

	// Binding 1: テクスチャサンプラー
	write_descriptor_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_sets[1].dstSet = this->descriptor_set;
	write_descriptor_sets[1].dstBinding = 1;
	write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_descriptor_sets[1].pImageInfo = this->texture.getDescriptorImageInfo();
	write_descriptor_sets[1].descriptorCount = 1;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);

}
void Object::allocateDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info) {
	if (vkAllocateDescriptorSets(device, &allocate_info, &this->descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets");
	}
}
void Object::updateUniformBuffer() {

}
