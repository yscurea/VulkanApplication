#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

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

		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions;
		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
		std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

#ifdef _DEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		VkInstanceCreateInfo create_info = initializer::getInstanceCreateInfo(&app_info, extensions);
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();

#ifdef _DEBUG
		debug.setupCreateInfo(&create_info);
#else
		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;
#endif

		if (vkCreateInstance(&create_info, nullptr, &this->instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	void Instance::deleteInstance() {
		vkDestroyInstance(this->instance, nullptr);
	}

#ifdef _DEBUG
	void Instance::setupDebug() {
		this->debug.setupDebugMessenger(&this->instance);
	}
	void Instance::deleteDebug() {

	}
#endif
}