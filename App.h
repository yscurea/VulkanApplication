#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <optional>

#include "Object.h"
#include "utils/Model.h"
#include "utils/DeviceQueueIndices.h"
#include "utils/SwapchainSupportDetails.h"

/// <summary>
/// 球体の複数描画する
/// </summary>
class App {
public:
	void run();
private:
	Model unique_model;
	void loadModel();
	uint32_t sphere_count = 100;
	std::vector<Object*> spheres;

	void prepareCommand();
	void updateUniformBuffers();

	void prepare();
	void render();

	void initVulkan();
	void cleanup();
protected:
	// ------------------------------ window ----------------------------------------
	GLFWwindow* window;
	int window_width = 800;
	int window_height = 600;
	void initWindow();
	void deleteWindow();
	// ウィンドウのサイズが変更されたときに呼び出されるコールバック関数
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	// ------------------------------ instance ----------------------------------------
	VkInstance instance;
	void createInstance();
	bool checkValidationLayerSupport();
	void deleteInstance();

#ifdef _DEBUG
	// ------------------------------ debug ----------------------------------------
	std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
	VkDebugUtilsMessengerEXT debug_messenger;
	void setupDebug();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void deleteDebug();
#endif


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
	VkFormat depth_image_format;
	void createDepthResources();
	void deleteDepthResources();

	size_t current_frame = 0;
	const size_t semaphore_size = 2;
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> images_in_flight;
	void createSyncObjects();
	// void deleteSyncObjects();


	// ------------------------------ render pass ----------------------------------------
	VkRenderPass render_pass;
	virtual void createRenderPass();
	void deleteRenderPass();

	// ------------------------------ command ----------------------------------------
	VkCommandPool command_pool;
	void createCommandPool();
	void deleteCommandPool();
	std::vector<VkCommandBuffer> command_buffers;
	void createCommandBuffers();
	void deleteCommandBuffers();



	// ------------------------------ pipeline ----------------------------------------
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	void createGraphcisPipeline();


	// ------------------------------ descriptor ----------------------------------------
	VkDescriptorPool descriptor_pool;
	void createDescriptorPool();
	VkDescriptorSetLayout descriptor_set_layout;
	void createDescriptorSetLayout();
	void createDescriptorSets();


	// ------------------------------ shader ----------------------------------------
	std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);


	// ------------------------------ utils ----------------------------------------
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
};

