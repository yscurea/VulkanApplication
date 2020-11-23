#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "utils/Model.h"
#include "utils/Buffer.h"
#include "utils/Texture.h"

// �����ł͑S�ẴI�u�W�F�N�g�͕`�悳���Ƃ����O��Ƃ���

class Object {
public:
	VkDescriptorSet* getDescriptorSet();
	Buffer* getUniformBuffer();
	Texture* getTexture();
	void draw(VkCommandBuffer command_buffer);
	// ���ʂ̃��f�����g�p����Ƃ��Ɏg��
	void setModel(Model* model);
	void loadModel(std::string model_path);
private:
	Model* model;
	VkDescriptorSet descriptor_set;
	Buffer uniform_buffer;
	Texture texture;
};

