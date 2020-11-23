#pragma once

#include <vulkan/vulkan.h>

#include "utils/Buffer.h"
#include "utils/Texture.h"

// ここでは全てのオブジェクトは描画されるという前提とする

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

