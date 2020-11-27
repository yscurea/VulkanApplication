#pragma once

class Debug {
	std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
	VkDebugUtilsMessengerEXT debug_messenger;
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
public:
	void setupDebugMessenger();
	void deleteDebugMessenger();
};