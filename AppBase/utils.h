#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>

namespace vulkan::utils {
	void checkError(VkResult result);

	namespace initializer {
		VkApplicationInfo getApplicationInfo(const char* application_name);
		VkInstanceCreateInfo getInstanceCreateInfo(VkApplicationInfo application_info, std::vector<const char*> extensions);

		VkSwapchainCreateInfoKHR getSwapchainCreateInfoKHR();
	}
}