#pragma once

#include <vulkan/vulkan.h>

namespace vulkan::utils {
	VkPipelineShaderStageCreateInfo getShaderStageCreateInfo(VkShaderModule shader_module, VkShaderStageFlagBits flag);
}