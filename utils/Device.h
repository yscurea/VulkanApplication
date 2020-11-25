#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "DeviceQueueIndices.h"
#include "SwapchainSupportDetails.h"

class Device {
private:
	// GPU
	VkPhysicalDevice physical_device;
	// �_���f�o�C�X
	VkDevice device;
	// �f�o�C�X�g��
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	// MSAA
	VkSampleCountFlagBits sample_count_falg_bits;
	// �O���t�B�b�N�X�L���[
	VkQueue graphics_queue;
	// �O���t�B�b�N�X�L���[�̃C���f�b�N�X
	std::optional<uint32_t> graphics_queue_index;
	// �v���[���g�L���[
	VkQueue present_queue;
	// �v���[���g�L���[�̃C���f�b�N�X
	std::optional<uint32_t> present_queue_index;
public:
	// GPU��I������
	void selectPhysicalDevice();
	// GPU���œK���ǂ����m�F����
	bool isDeviceSuitable(VkPhysicalDevice physical_device);
	// �f�o�C�X�g�����Ή����Ă��邩�ǂ����m�F����
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	// �_���f�o�C�X�𐶐�����
	void createLogicalDevice();
	// �_���f�o�C�X��j������
	void deleteLogicalDevice();
	// �I������Ă���GPU�̃L���[���擾����
	DeviceQueueIndices findDeviceQueue(VkPhysicalDevice physical_device);



	SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
};