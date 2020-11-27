#pragma once

#include <vulkan/vulkan.h>


class Instance {
	VkInstance instance;
public:
	void createInstance();
	void deleteInstance();
};