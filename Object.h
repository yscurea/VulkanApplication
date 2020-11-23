#pragma once

#include <vulkan/vulkan.h>

#include "utils/Buffer.h"
#include "utils/Texture.h"

// �����ł͑S�ẴI�u�W�F�N�g�͕`�悳���Ƃ����O��Ƃ���

class Object {
public:
	VkDescriptorSet* getDescriptorSet();
	Buffer* getUniformBuffer();
	Texture* getTexture();
private:
	VkDescriptorSet descriptor_set;
	Buffer uniform_buffer;
	Texture texture;
};

