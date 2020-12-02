#include "./Debug.h"

#include <iostream>
// #include <stdexcept>

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan::base::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer:\t" << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

namespace vulkan::base {


	VkDebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfoEXT() {
		VkDebugUtilsMessengerCreateInfoEXT create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = debugCallback;
		return create_info;
	}

	void setupCreateInfo(VkInstanceCreateInfo* create_info) {
		VkDebugUtilsMessengerCreateInfoEXT debug_create_info = getDebugUtilsMessengerCreateInfoEXT();
		create_info->enabledLayerCount = static_cast<uint32_t>(this->validation_layers.size());
		create_info->ppEnabledLayerNames = this->validation_layers.data();
		populateDebugMessengerCreateInfo(debug_create_info);
		create_info->pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	}
}
