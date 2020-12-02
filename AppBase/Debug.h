#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace vulkan::base {
	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	class Debug {
		std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
		VkDebugUtilsMessengerEXT debug_messenger;
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	public:
		void setupDebugMessenger();
		void deleteDebugMessenger();
		bool checkValidationLayerSupport();
		void setupCreateInfo(VkInstanceCreateInfo* create_info);
		// VkDebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfoEXT();
	};
}