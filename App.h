#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <optional>


#include "utils/utils.h"

#include "Object.h"
#include "utils/Vertex.h"
#include "utils/DeviceQueueIndices.h"
#include "utils/SwapchainSupportDetails.h"
#include "Camera.h"

// todo: 各々が密接すぎてクラス分けが難しいけどクラスわけする
	// 理想は ----- で区切っているものを全てクラスとして分離する
// todo: vulkanの初期化で共通する部分を親クラスとする

/// <summary>
/// 球体の大量に描画する
/// </summary>
class App {
public:
	App(uint32_t sphere_count);
	void run();
private:
	void loadModel();
	Camera camera;
	glm::vec3 light_position = glm::vec3(10.0f, 10.0f, 10.0f);
	uint32_t sphere_count = 1000;
	std::vector<Object> spheres;

	void prepareCommand();

	void prepare();
	void render();

	void initVulkan();
	void cleanup();



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



	// ------------------------------ debug ----------------------------------------
#ifdef _DEBUG
	std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
	VkDebugUtilsMessengerEXT debug_messenger;
	void setupDebug();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void deleteDebugMessenger();
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
	void deleteLogicalDevice();
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
	// void deleteSwapchainImages();
	std::vector<VkImageView> swapchain_image_views;
	void createSwapchainImageViews();
	void deleteSwapchainImageViews();
	VkExtent2D swapchain_extent;

	// framebuffer
	std::vector<VkFramebuffer> swapchain_framebuffers;
	bool framebuffer_resized = false;
	void createSwapchainFrameBuffers();
	void deleteSwapchainFrameBuffers();
	// offscreen framebuffer
	void createOffscreenFrameBuffer();
	void deleteOffscreenFrameBuffer();

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
	void deleteSyncObjects();

	// ------------------------------ render pass ----------------------------------------

	// 通常のレンダリングパス
	VkRenderPass render_pass;
	virtual void createRenderPass();
	void deleteRenderPass();

	// オフスクリーンレンダリングパス（光源からみた深度値計算）
	VkRenderPass offscreen_render_pass;
	void createOffscreenRenderPass();
	void deleteOffscreenRenderPass();

	// ------------------------------ command ----------------------------------------

	VkCommandPool command_pool;
	void createCommandPool();
	void deleteCommandPool();
	std::vector<VkCommandBuffer> command_buffers;
	void createCommandBuffers();
	void deleteCommandBuffers();

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer command_buffer);

	// ------------------------------ pipeline ----------------------------------------

	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	void createGraphcisPipeline();

	VkPipeline offscreen_pipeline;
	void createOffscreenPipeline();
	void preparePipelines();



	// ------------------------------ descriptor ----------------------------------------

	void prepareDescriptor();
	void deleteDescriptor();
	VkDescriptorPool descriptor_pool;
	void createDescriptorPool();
	VkDescriptorSetLayout descriptor_set_layout;
	void createDescriptorSetLayout();
	void createDescriptorSets();

	// ------------------------------ buffers ----------------------------------------

	// 今回は頂点もテクスチャも共通のものを使用するため単一

	// 頂点バッファ
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;
	void createVertexBuffer();
	void deleteVertexBuffer();
	void createIndexBuffer();
	void deleteIndexBuffer();
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	// テクスチャ
	std::string texture_file_path = "textures/sample.png";
	uint32_t mip_levels;
	VkImage texture_image;
	VkDeviceMemory texture_image_memory;
	VkImageView texture_image_view;
	VkSampler texture_sampler;

	void prepareTexture();
	void deleteTexture();

	void createTextureImage(std::string texture_file_path);
	void createTextureImageView();
	void createTextureSampler();

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	// ユニフォームバッファ（定数バッファ）
	void createUniformBuffers();
	// void createUniformBufferOffscreen();
	void updateUniformBuffers();
	void updateUniformBufferOffscreen();

	// ------------------------------ shader ----------------------------------------

	std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);


	// ------------------------------ utils ----------------------------------------

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
};

