#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "SwapchainSupportDetails.h"

class Swapchain {
private:
	VkSwapchainKHR swapchain;
	VkFormat swapchain_image_format;

	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;

	VkExtent2D swapchain_extent;

	// framebuffer
	std::vector<VkFramebuffer> swapchain_framebuffers;
	bool framebuffer_resized = false;

	VkImage color_image;
	VkDeviceMemory color_image_memory;
	VkImageView color_image_view;

	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;
	VkFormat depth_image_format;

	size_t current_frame = 0;
	const size_t semaphore_size = 2;
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> images_in_flight;
public:
	void createSwapchain();
	void deleteSwapchain();
	void createSwapchainImages();
	void createSwapchainImageViews();
	void deleteSwapchainImageViews();
	void createSwapchainFrameBuffers();
	void deleteSwapchainFrameBuffers();

	void createColorResources();
	void deleteColorResources();

	void createDepthResources();
	void deleteDepthResources();
	void createSyncObjects();
	void deleteSyncObjects();
};