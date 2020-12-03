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

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}


	void Debug::setupCreateInfo(VkInstanceCreateInfo* create_info) {
		VkDebugUtilsMessengerCreateInfoEXT debug_create_info = getDebugUtilsMessengerCreateInfoEXT();
		create_info->enabledLayerCount = static_cast<uint32_t>(this->validation_layers.size());
		create_info->ppEnabledLayerNames = this->validation_layers.data();
		create_info->pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	}

	void Debug::setupDebugMessenger(VkInstance& instance) {
		VkDebugUtilsMessengerCreateInfoEXT create_info;
		populateDebugMessengerCreateInfo(create_info);
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		if (CreateDebugUtilsMessengerEXT(this->instance, &create_info, nullptr, &this->debug_messenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
}
