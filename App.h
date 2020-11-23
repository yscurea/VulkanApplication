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
	void loadModel();
	uint32_t sphere_count = 100;
	std::vector<Object*> spheres;

	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	void createGraphcisPipeline();

	VkDescriptorPool descriptor_pool;
	void createDescriptorPool();
	VkDescriptorSetLayout descriptor_set_layout;
	void createDescriptorSetLayout();
	void createDescriptorSets();

	void prepareCommand();
	void updateUniformBuffers();

	void prepare();
	void render();
	void cleanup();
};

