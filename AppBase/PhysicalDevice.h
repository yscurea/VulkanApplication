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
	// GPUを選択する
	void selectPhysicalDevice();
	// GPUが最適かどうか確認する
	bool isDeviceSuitable(VkPhysicalDevice physical_device);
	// queueを更新する
	void updateDeviceQueue();
	// デバイス拡張が対応しているかどうか確認する
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
};