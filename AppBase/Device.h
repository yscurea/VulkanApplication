#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "DeviceQueueIndices.h"
#include "SwapchainSupportDetails.h"

class Device {
private:
	// GPU
	VkPhysicalDevice physical_device;
	// 論理デバイス
	VkDevice device;
	// デバイス拡張
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	// MSAA
	VkSampleCountFlagBits sample_count_falg_bits;
	// グラフィックスキュー
	VkQueue graphics_queue;
	// グラフィックスキューのインデックス
	std::optional<uint32_t> graphics_queue_index;
	// プレゼントキュー
	VkQueue present_queue;
	// プレゼントキューのインデックス
	std::optional<uint32_t> present_queue_index;
public:
	// GPUを選択する
	void selectPhysicalDevice();
	// GPUが最適かどうか確認する
	bool isDeviceSuitable(VkPhysicalDevice physical_device);
	// デバイス拡張が対応しているかどうか確認する
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	// 論理デバイスを生成する
	void createLogicalDevice();
	// 論理デバイスを破棄する
	void deleteLogicalDevice();
	// 選択されているGPUのキューを取得する
	DeviceQueueIndices findDeviceQueue(VkPhysicalDevice physical_device);



	SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
};