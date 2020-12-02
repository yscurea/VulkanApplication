#include "./Instance.h"

#include "utils.h"


using namespace vulkan::utils;

namespace vulkan::base {
	Instance::Instance() {
	}

	Instance::~Instance() {
		this->deleteInstance();
	}

	void Instance::createInstance() {
#ifdef _DEBUG
		if (this->debug.checkValidationLayerSupport() == false) {
			throw std::runtime_error("validation layers requested. but not available!");
		}
#endif
		VkApplicationInfo app_info = initializer::getApplicationInfo("AppName");

		VkInstanceCreateInfo create_info = initializer::getInstanceCreateInfo(&app_info, extensions);
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;

		std::vector<const char*> extensions;
		{
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			std::vector<const char*> tmp_extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef _DEBUG
			tmp_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
			extensions = tmp_extensions;
		}
		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();

#ifdef _DEBUG
		debug.setupCreateInfo(&create_info);
		VkDebugUtilsMessengerCreateInfoEXT debug_create_info = debug.getDebugUtilsMessengerCreateInfoEXT();
		create_info.enabledLayerCount = static_cast<uint32_t>(this->validation_layers.size());
		create_info.ppEnabledLayerNames = this->validation_layers.data();
		populateDebugMessengerCreateInfo(debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
#else
		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;
#endif

		VkInstanceCreateInfo create_info = initializer::getInstanceCreateInfo(&app_info, extensions);

		if (vkCreateInstance(&create_info, nullptr, &this->instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	void Instance::deleteInstance() {
		vkDestroyInstance(this->instance, nullptr);
	}

}