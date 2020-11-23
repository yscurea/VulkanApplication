#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vector>

// è„éËÇ≠ÉNÉâÉXÇÌÇØÇµÇΩÇ¢ÇØÇ«ÇªÇÍÇºÇÍÇ™ñßê⁄Ç…ä÷åWÇµÇ∑Ç¨ÇƒÇƒÇÌÇ©ÇÁÇÒ


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
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	// ------------------------------ instance ----------------------------------------
	VkInstance instance;
	void createInstance();
	void deleteInstance();

	// ------------------------------ surface ----------------------------------------
	VkSurfaceKHR surface;
	void createSurface();
	void deleteSurface();

	// ------------------------------ device ----------------------------------------
	VkPhysicalDevice physical_device;
	void selectPhysicalDevice();
	VkDevice device;
	void createLogicalDevice();
	VkQueue graphics_queue;
	uint32_t graphics_queue_index;
	VkQueue present_queue;
	uint32_t present_queue_index;


	// ------------------------------ swapchain ----------------------------------------
	VkSwapchainKHR swapchain;
	VkFormat swapchain_image_format;
	void createSwapchain();
	void deleteSwapchain();
	std::vector<VkImage> swapchain_images;
	void createSwapchainImages();
	std::vector<VkImageView> swapchain_image_views;
	void createSwapchainImageViews();
	VkExtent2D swapchain_extent;
	// framebuffer
	std::vector<VkFramebuffer> swapchain_framebuffers;
	bool framebuffer_resized = false;
	void createSwapchainFrameBuffers();
	VkImage color_image;
	VkDeviceMemory color_image_memory;
	VkImageView color_image_view;
	void createColorResources();
	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;
	void createDepthResources();
	size_t current_frame = 0;
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> images_in_flight;
	void createSyncObject();


	// ------------------------------ render pass ----------------------------------------
	VkRenderPass render_pass;
	void createRenderPass();

	// ------------------------------ command ----------------------------------------
	VkCommandPool command_pool;
	void createCommandPool();
	void deleteCommandPool();
	std::vector<VkCommandBuffer> command_buffers;
	void createCommandBuffers();
	void deleteCommandBuffers();
};
