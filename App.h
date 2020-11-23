#pragma once

#include "AppBase.h"

#include "Object.h"
#include "utils/Model.h"

/// <summary>
/// ãÖëÃÇÃï°êîï`âÊÇ∑ÇÈ
/// </summary>
class App : public AppBase {
public:
	void run();
private:
	Model unique_model;

	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	void createGraphcisPipeline();

	VkDescriptorPool descriptor_pool;
	void createDescriptorPool();
	VkDescriptorSetLayout descriptor_set_layout;
	void createDescriptorSetLayout();
	std::vector<VkDescriptorSet> descriptor_sets;
	void createDescriptorSets();

	uint32_t sphere_count = 100;
	std::vector<Object*> spheres;

	void prepare();
	void loadModel();
	void prepareCommand();

	void updateUniformBuffers();

	void render();
};

