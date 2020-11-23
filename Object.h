#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "utils/Model.h"
#include "utils/Buffer.h"
#include "utils/Texture.h"

// ここでは全てのオブジェクトは描画されるという前提とする

class Object {
public:
	void draw(VkCommandBuffer command_buffer);
	void allocateDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info);
	void bindDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);
	void setModel(Model* model); // 共通のモデルを使用するときに使う
	void loadModel(std::string model_path); // 新規読み込みに使う
	void updateUniformBuffer();
private:
	Model* model;
	VkDescriptorSet descriptor_set;
	Buffer uniform_buffer;
	Texture texture;
};

