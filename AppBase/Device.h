#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "PhysicalDevice.h"

#include "DeviceQueueIndices.h"

class Device {
private:
	PhysicalDevice physical_device;
	// �_���f�o�C�X
	VkDevice device;
	// �f�o�C�X�g��
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
public:
	// �_���f�o�C�X�𐶐�����
	void createLogicalDevice();
	// �_���f�o�C�X��j������
	void deleteLogicalDevice();

};