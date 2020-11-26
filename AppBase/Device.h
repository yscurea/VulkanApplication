#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "PhysicalDevice.h"

#include "DeviceQueueIndices.h"

class Device {
private:
	PhysicalDevice physical_device;
	// 論理デバイス
	VkDevice device;
	// デバイス拡張
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
public:
	// 論理デバイスを生成する
	void createLogicalDevice();
	// 論理デバイスを破棄する
	void deleteLogicalDevice();

};