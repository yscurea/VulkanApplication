#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <optional>

#include "utils/DeviceQueueIndices.h"
#include "utils/SwapchainSupportDetails.h"

// 上手くクラスわけしたいけどそれぞれが密接に関係しすぎててわからん

class AppBase
{
public:
	void initVulkan();
	void cleanup();
	int window_width = 800;
	int window_height = 600;
protected:
	// ------------------------------ window ----------------------------------------
	GLFWwindow* window;
	void createWindow();
	void deleteWindow();
	// ウィンドウのサイズが変更されたときに呼び出されるコールバック関数
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	// ------------------------------ instance ----------------------------------------
	VkInstance instance;
	void createInstance();
	void deleteInstance();

	// ------------------------------ debug ----------------------------------------
	std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
	void setupDebug();
	void deleteDebug();


	// ------------------------------ surface ----------------------------------------
	VkSurfaceKHR surface;
	void createSurface();
	void deleteSurface();

	// ------------------------------ device ----------------------------------------
	VkPhysicalDevice physical_device;
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkSampleCountFlagBits sample_count_falg_bits;
	void selectPhysicalDevice();
	DeviceQueueIndices findDeviceQueue(VkPhysicalDevice physical_device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice physical_device);
	VkDevice device;
	void createLogicalDevice();
	VkQueue graphics_queue;
	std::optional<uint32_t> graphics_queue_index;
	VkQueue present_queue;
	std::optional<uint32_t> present_queue_index;


	// ------------------------------ swapchain ----------------------------------------
	VkSwapchainKHR swapchain;
	VkFormat swapchain_image_format;
	void createSwapchain();
	void deleteSwapchain();
	std::vector<VkImage> swapchain_images;
	void createSwapchainImages();
	void deleteSwapchainImages();
	std::vector<VkImageView> swapchain_image_views;
	void createSwapchainImageViews();
	void deleteSwapchainImageViews();
	VkExtent2D swapchain_extent;
	// framebuffer
	std::vector<VkFramebuffer> swapchain_framebuffers;
	bool framebuffer_resized = false;
	void createSwapchainFrameBuffers();
	void deleteSwapchainFrameBuffers();

	VkImage color_image;
	VkDeviceMemory color_image_memory;
	VkImageView color_image_view;
	void createColorResources();
	void deleteColorResources();

	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;
	void createDepthResources();
	void deleteDepthResources();

	size_t current_frame = 0;
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> images_in_flight;
	void createSyncObjects();
	void deleteSyncObjects();


	// ------------------------------ render pass ----------------------------------------
	VkRenderPass render_pass;
	void createRenderPass();
	void deleteRenderPass();

	// ------------------------------ command ----------------------------------------
	VkCommandPool command_pool;
	void createCommandPool();
	void deleteCommandPool();
	std::vector<VkCommandBuffer> command_buffers;
	void createCommandBuffers();
	void deleteCommandBuffers();
};
