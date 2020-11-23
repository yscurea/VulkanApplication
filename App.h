#pragma once

#include "AppBase.h"

// todo : �I�u�W�F�N�g�N���X��ʃt�@�C���Ɉړ�
class Object {};

class App : public AppBase {
public:
	void run();
private:
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	void createGraphcisPipeline();

	VkDescriptorPool descriptor_pool;
	void createDescriptorPool();
	VkDescriptorSetLayout descriptor_set_layout;
	void createDescriptorSetLayout();
	std::vector<VkDescriptorSet> descriptor_sets;
	void createDescriptorSets();

	std::vector<Object*> spheres;

	void prepare();
	void loadModel();
	void prepareCommand();

	void updateUniformBuffers();

	void render();
};

