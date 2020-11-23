#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "utils/Model.h"
#include "utils/Buffer.h"
#include "utils/Texture.h"

// ここでは全てのオブジェクトは描画されるという前提とする

class Object {
public:
	VkDescriptorSet* getDescriptorSet();
	Buffer* getUniformBuffer();
	Texture* getTexture();
	void draw(VkCommandBuffer command_buffer);
	// 共通のモデルを使用するときに使う
	void setModel(Model* model);
	void loadModel(std::string model_path);
private:
	Model* model;
	VkDescriptorSet descriptor_set;
	Buffer uniform_buffer;
	Texture texture;
};

