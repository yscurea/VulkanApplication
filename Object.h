#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "utils/Model.h"
#include "utils/Buffer.h"
#include "utils/Texture.h"

// �����ł͑S�ẴI�u�W�F�N�g�͕`�悳���Ƃ����O��Ƃ���

class Object {
public:
	void draw(VkCommandBuffer command_buffer);
	void allocateDescriptorSets(VkDevice& device, VkDescriptorSetAllocateInfo allocate_info);
	void bindDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);
	void setModel(Model* model); // ���ʂ̃��f�����g�p����Ƃ��Ɏg��
	void loadModel(std::string model_path); // �V�K�ǂݍ��݂Ɏg��
	void updateUniformBuffer();
private:
	Model* model;
	VkDescriptorSet descriptor_set;
	Buffer uniform_buffer;
	Texture texture;
};

