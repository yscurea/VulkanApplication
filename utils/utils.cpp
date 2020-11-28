#include "utils.h"


namespace vulkan::utils {
	VkPipelineShaderStageCreateInfo getShaderStageCreateInfo(VkShaderModule shader_module, VkShaderStageFlagBits flag) {
		VkPipelineShaderStageCreateInfo shader_stage_create_info{};
		shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_create_info.stage = flag;
		shader_stage_create_info.module = shader_module;
		shader_stage_create_info.pName = "main";
		return shader_stage_create_info;
	}
}