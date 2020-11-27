#pragma once

#include <optional>

class PhysicalDevice {
	// GPU
	VkPhysicalDevice physical_device;
	VkQueue present_queue;
	std::optional<uint32_t> present_queue_index;

	VkQueue graphics_ququq;
	std::optional<uint32_t> graphics_queue_index;

	VkQueue compute_queue;
	std::optional<uint32_t> compute_queue_index;

	// MSAA
	VkSampleCountFlagBits sample_count_falg_bits;
public:
	// GPU��I������
	void selectPhysicalDevice();
	// GPU���œK���ǂ����m�F����
	bool isDeviceSuitable(VkPhysicalDevice physical_device);
	// queue���X�V����
	void updateDeviceQueue();
	// �f�o�C�X�g�����Ή����Ă��邩�ǂ����m�F����
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
};