#include "App.h"

#include <array>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <set>
#include <array>
#include <algorithm>
#include <random>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "utils/UniformBufferObject.h"

using std::cout;
using std::endl;


App::App(uint32_t sphere_count) {
	this->sphere_count = sphere_count;
}

// vulkanの基本的な初期化
void App::initVulkan() {
	this->initWindow();

	this->createInstance();
#ifdef _DEBUG
	this->setupDebug();
#endif
	this->createSurface();
	this->selectPhysicalDevice();
	this->createLogicalDevice();
	this->createSwapchain();
	this->createRenderPass();

	this->createSyncObjects();
}


// 実行
void App::run() {
	this->initVulkan();
	this->prepare();
	while (!glfwWindowShouldClose(this->window)) {
		glfwPollEvents();
		this->render();
	}
	vkDeviceWaitIdle(this->device);
	this->cleanup();
}

// アプリケーションの事前準備
void App::prepare() {
	this->loadModel();
	this->spheres.resize(this->sphere_count);
	this->createDescriptorSetLayout();

	// パイプライン作成
	this->preparePipelines();

	// コマンドプール作成
	this->createCommandPool();

	this->createColorResources();
	this->createDepthResources();
	this->createSwapchainFrameBuffers();

	this->createOffscreenFrameBuffer();

	// 各種リソース作成
	this->prepareTexture();
	this->createVertexBuffer();
	this->createIndexBuffer();
	this->createUniformBuffers();


	// デスクリプタ準備
	this->createDescriptorPool();
	this->createDescriptorSets();

	// コマンドバッファ作成
	this->createCommandBuffers();
	// コマンド記録
	this->prepareCommand();
}
void App::render() {
	vkWaitForFences(this->device, 1, &this->in_flight_fences[this->current_frame], VK_TRUE, UINT64_MAX);

	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(this->device, this->swapchain, UINT64_MAX, this->image_available_semaphores[this->current_frame], VK_NULL_HANDLE, &image_index);

	this->updateUniformBuffers();
	this->updateUniformBufferOffscreen();

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { this->image_available_semaphores[current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &this->command_buffers[image_index];

	VkSemaphore signal_semaphores[] = { this->render_finished_semaphores[current_frame] };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	vkResetFences(this->device, 1, &this->in_flight_fences[current_frame]);

	if (vkQueueSubmit(this->graphics_queue, 1, &submit_info, this->in_flight_fences[current_frame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}


	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swapchains[] = { this->swapchain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;

	present_info.pImageIndices = &image_index;

	result = vkQueuePresentKHR(this->present_queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->framebuffer_resized) {
		this->framebuffer_resized = false;
		// this->recreateSwapchain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	this->current_frame = (this->current_frame + 1) % semaphore_size;
}
void App::cleanup() {
	this->deleteSwapchain();
	this->deleteTexture();
	this->deleteDescriptor();
	this->deleteIndexBuffer();
	this->deleteVertexBuffer();
	this->deleteSyncObjects();
	this->deleteCommandPool();
	this->deleteLogicalDevice();
#ifdef _DEBUG
	this->deleteDebugMessenger();
#endif
	this->deleteSurface();
	this->deleteInstance();
	this->deleteWindow();
}


// ------------------------------ utils ----------------------------------------
uint32_t App::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(this->physical_device, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");
}
void App::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(this->device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(this->device, buffer, &memRequirements);

	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = memRequirements.size;
	alloc_info.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);


	if (vkAllocateMemory(this->device, &alloc_info, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(this->device, buffer, bufferMemory, 0);
}
void App::createImage(uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory) {
	VkImageCreateInfo image_info{};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = mip_levels;
	image_info.arrayLayers = 1;
	image_info.format = format;
	image_info.tiling = tiling;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = usage;
	image_info.samples = num_samples;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(this->device, &image_info, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(this->device, image, &mem_requirements);

	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, properties);

	if (vkAllocateMemory(this->device, &alloc_info, nullptr, &image_memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}
	vkBindImageMemory(this->device, image, image_memory, 0);
}
VkImageView App::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) {
	VkImageViewCreateInfo view_info{};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect_flags;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = mip_levels;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(this->device, &view_info, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}
VkFormat findSupportedFormat(VkPhysicalDevice physical_device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}
VkFormat findDepthFormat(VkPhysicalDevice physical_device) {
	return findSupportedFormat(
		physical_device,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}




// ------------------------------ window ----------------------------------------
void App::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	this->window = glfwCreateWindow(this->window_width, this->window_height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(this->window, this);
	glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);
}
void App::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->framebuffer_resized = true;

}
void App::deleteWindow() {
	glfwDestroyWindow(this->window);
	glfwTerminate();
}


// ------------------------------ instance ----------------------------------------
#ifdef _DEBUG
bool App::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : this->validation_layers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}
#endif

void App::createInstance() {
#ifdef _DEBUG
	if (checkValidationLayerSupport() == false) {
		throw std::runtime_error("validation layers requested. but not available!");
	}
#endif
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Hello Triangle";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	std::vector<const char*> extensions;
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> tmp_extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef _DEBUG
		tmp_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		extensions = tmp_extensions;
	}

	create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
#ifdef _DEBUG
	create_info.enabledLayerCount = static_cast<uint32_t>(this->validation_layers.size());
	create_info.ppEnabledLayerNames = this->validation_layers.data();
	populateDebugMessengerCreateInfo(debug_create_info);
	create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
#else
	create_info.enabledLayerCount = 0;
	create_info.pNext = nullptr;
#endif

	if (vkCreateInstance(&create_info, nullptr, &this->instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}
void App::deleteInstance() {
	vkDestroyInstance(this->instance, nullptr);
}


// ------------------------------ debug ----------------------------------------
#ifdef _DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer:\t" << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}
void App::setupDebug() {
	VkDebugUtilsMessengerCreateInfoEXT create_info;
	populateDebugMessengerCreateInfo(create_info);

	if (CreateDebugUtilsMessengerEXT(this->instance, &create_info, nullptr, &this->debug_messenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}
void App::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}
void App::deleteDebugMessenger() {
	DestroyDebugUtilsMessengerEXT(this->instance, this->debug_messenger, nullptr);
}
#endif

// ------------------------------ surface ----------------------------------------
void App::createSurface() {
	if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface");
	}
}
void App::deleteSurface() {
	vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
}

// ------------------------------ device ----------------------------------------
DeviceQueueIndices App::findDeviceQueue(VkPhysicalDevice physical_device) {
	DeviceQueueIndices indices;

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_queue_index = i;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, this->surface, &presentSupport);
		if (presentSupport) {
			indices.present_queue_index = i;
		}
		if (indices.isComplete()) {
			break;
		}
		i++;
	}
	return indices;
}
bool App::checkDeviceExtensionSupport(VkPhysicalDevice physical_device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(this->device_extensions.begin(), this->device_extensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}
SwapChainSupportDetails App::querySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}
bool App::isDeviceSuitable(VkPhysicalDevice physical_device) {
	uint32_t graphics_queue_index = UINT32_MAX;
	uint32_t present_queue_index = UINT32_MAX;
	DeviceQueueIndices indices = this->findDeviceQueue(physical_device);

	bool extensions_supported = this->checkDeviceExtensionSupport(physical_device);

	bool swapchain_adequate = false;
	if (extensions_supported) {
		SwapChainSupportDetails swapchain_support = querySwapChainSupport(physical_device);
		swapchain_adequate = !swapchain_support.formats.empty() && !swapchain_support.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supported_features;
	vkGetPhysicalDeviceFeatures(physical_device, &supported_features);

	return indices.isComplete() && extensions_supported && swapchain_adequate && supported_features.samplerAnisotropy;
}
static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physical_device) {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physical_device, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}
void App::selectPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			this->physical_device = device;
			this->sample_count_falg_bits = getMaxUsableSampleCount(this->physical_device);
			break;
		}
	}
	if (this->physical_device == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	DeviceQueueIndices indices = findDeviceQueue(this->physical_device);
	this->graphics_queue_index = indices.graphics_queue_index.value();
	this->present_queue_index = indices.present_queue_index.value();
}
void App::createLogicalDevice() {
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	std::set<uint32_t> unique_queue_families = { this->graphics_queue_index.value(),this->present_queue_index.value() };

	float queue_priority = 1.0f;
	for (uint32_t queue_family : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_family;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures device_features{};
	device_features.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	create_info.pQueueCreateInfos = queue_create_infos.data();

	create_info.pEnabledFeatures = &device_features;

	create_info.enabledExtensionCount = static_cast<uint32_t>(this->device_extensions.size());
	create_info.ppEnabledExtensionNames = this->device_extensions.data();

#ifdef _DEBUG
	create_info.enabledLayerCount = static_cast<uint32_t>(this->validation_layers.size());
	create_info.ppEnabledLayerNames = this->validation_layers.data();
#else
	create_info.enabledLayerCount = 0;
#endif

	if (vkCreateDevice(this->physical_device, &create_info, nullptr, &this->device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(this->device, this->graphics_queue_index.value(), 0, &this->graphics_queue);
	vkGetDeviceQueue(this->device, this->present_queue_index.value(), 0, &this->present_queue);
}
void App::deleteLogicalDevice() {
	vkDestroyDevice(this->device, nullptr);
}


// ------------------------------ swapchain ----------------------------------------
static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
	for (const auto& availableFormat : available_formats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return available_formats[0];
}
static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
	for (const auto& availablePresentMode : available_present_modes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}
static VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actual_extent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

		return actual_extent;
	}
}

void App::createSwapchain() {
	SwapChainSupportDetails swapchain_support = querySwapChainSupport(this->physical_device);

	VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swapchain_support.formats);
	VkPresentModeKHR present_mode = chooseSwapPresentMode(swapchain_support.presentModes);
	VkExtent2D extent = chooseSwapExtent(this->window, swapchain_support.capabilities);

	uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
	if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount) {
		image_count = swapchain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = this->surface;

	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { this->graphics_queue_index.value(), this->present_queue_index.value() };

	if (this->graphics_queue_index != this->present_queue_index) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	create_info.preTransform = swapchain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(this->device, &create_info, nullptr, &this->swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(this->device, this->swapchain, &image_count, nullptr);
	swapchain_images.resize(image_count);
	vkGetSwapchainImagesKHR(this->device, this->swapchain, &image_count, this->swapchain_images.data());

	this->swapchain_image_format = surface_format.format;
	this->swapchain_extent = extent;

	this->createSwapchainImageViews();
}
void App::deleteSwapchain() {
	vkDestroyImageView(this->device, this->depth_image_view, nullptr);
	vkDestroyImage(this->device, this->depth_image, nullptr);
	vkFreeMemory(this->device, this->depth_image_memory, nullptr);

	for (auto framebuffer : swapchain_framebuffers) {
		vkDestroyFramebuffer(this->device, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(this->device, command_pool, static_cast<uint32_t>(this->command_buffers.size()), this->command_buffers.data());

	vkDestroyPipeline(this->device, this->graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(this->device, this->pipeline_layout, nullptr);
	vkDestroyRenderPass(this->device, this->render_pass, nullptr);

	for (auto imageView : this->swapchain_image_views) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);

	for (auto sphere : this->spheres) {
		sphere.deleteUniformBuffer(this->device);
	}

	vkDestroyDescriptorPool(this->device, this->descriptor_pool, nullptr);
}
void App::createSwapchainImageViews() {
	this->swapchain_image_views.resize(this->swapchain_images.size());

	for (uint32_t i = 0; i < this->swapchain_images.size(); i++) {
		this->swapchain_image_views[i] = createImageView(this->swapchain_images[i], this->swapchain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}
void App::deleteSwapchainImageViews() {
	for (auto image_view : this->swapchain_image_views) {
		vkDestroyImageView(this->device, image_view, nullptr);
	}
}
void App::createSwapchainFrameBuffers() {
	this->swapchain_framebuffers.resize(this->swapchain_image_views.size());

	for (size_t i = 0; i < this->swapchain_image_views.size(); i++) {
		std::array<VkImageView, 3> attachments = {
			this->color_image_view,
			this->depth_image_view,
			this->swapchain_image_views[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->render_pass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = this->swapchain_extent.width;
		framebufferInfo.height = this->swapchain_extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &this->swapchain_framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}
void App::deleteSwapchainFrameBuffers() {
	for (auto framebuffer : this->swapchain_framebuffers) {
		vkDestroyFramebuffer(this->device, framebuffer, nullptr);
	}
}
void App::createOffscreenFrameBuffer() {
	// todo : ここから再開

	offscreen_pass.width = SHADOWMAP_DIM;
	offscreen_pass.height = SHADOWMAP_DIM;

	// For shadow mapping we only need a depth attachment
	VkImageCreateInfo image_create_info = vks::initializers::imageCreateInfo();
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.extent.width = offscreenPass.width;
	image_create_info.extent.height = offscreenPass.height;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.format = DEPTH_FORMAT;																// Depth stencil attachment
	image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;		// We will sample directly from the depth attachment for the shadow mapping
	if (vkCreateImage(device, &image_create_info, nullptr, &offscreenPass.depth.image) != VK_SUCCESS) {
		throw std::runtime_error("failed to creat image in creating offscreen render pass");
	}

	VkMemoryAllocateInfo memory_allocate_info = vks::initializers::memoryAllocateInfo();
	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device, offscreenPass.depth.image, &memory_requirements);
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = vulkanDevice->getMemoryType(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(this->device, &memory_allocate_info, nullptr, &offscreenPass.depth.mem) = VK_SUCCESS) {
		throw std::runtime_error("failed to allocate memory in createOffscreenFrameBuffer");
	}
	if (vkBindImageMemory(this->device, offscreenPass.depth.image, offscreenPass.depth.mem, 0) = VK_SUCCESS) {
		throw std::runtime_error("failed to bind memory in createOffscreenFrameBuffer");
	}

	VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = DEPTH_FORMAT;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;
	depthStencilView.image = offscreenPass.depth.image;
	if (vkCreateImageView(device, &depthStencilView, nullptr, &offscreenPass.depth.view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view in createOffscreenFrameBuffers");
	}

	VkFilter shadowmap_filter = vks::tools::formatIsFilterable(this->physical_device, DEPTH_FORMAT, VK_IMAGE_TILING_OPTIMAL) ?
		DEFAULT_SHADOWMAP_FILTER :
		VK_FILTER_NEAREST;
	VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
	sampler.magFilter = shadowmap_filter;
	sampler.minFilter = shadowmap_filter;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	if (vkCreateSampler(device, &sampler, nullptr, &offscreenPass.depthSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create sampler in createOffscreenFrameBuffers");
	}

	this->createOffscreenRenderPass();

	// Create frame buffer
	VkFramebufferCreateInfo framebuffer_create_info = vks::initializers::framebufferCreateInfo();
	framebuffer_create_info.renderPass = offscreenPass.renderPass;
	framebuffer_create_info.attachmentCount = 1;
	framebuffer_create_info.pAttachments = &offscreenPass.depth.view;
	framebuffer_create_info.width = offscreenPass.width;
	framebuffer_create_info.height = offscreenPass.height;
	framebuffer_create_info.layers = 1;

	if (vkCreateFramebuffer(this->device, &framebuffer_create_info, nullptr, &offscreenPass.frameBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create offscreen_framebuffer");
	}
}
void App::deleteOffscreenFrameBuffer() {
}
void App::createColorResources() {
	VkFormat colorFormat = swapchain_image_format;

	createImage(this->swapchain_extent.width, this->swapchain_extent.height, 1, this->sample_count_falg_bits, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->color_image, this->color_image_memory);
	this->color_image_view = createImageView(this->color_image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}
void App::deleteColorResources() {
	vkDestroyImageView(this->device, this->color_image_view, nullptr);
	vkDestroyImage(this->device, this->color_image, nullptr);
	vkFreeMemory(this->device, this->color_image_memory, nullptr);
}
void App::createDepthResources() {
	VkFormat depthFormat = findDepthFormat(this->physical_device);

	createImage(this->swapchain_extent.width, this->swapchain_extent.height, 1, this->sample_count_falg_bits, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->depth_image, this->depth_image_memory);
	this->depth_image_view = createImageView(this->depth_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}
void App::deleteDepthResources() {
	vkDestroyImageView(device, depth_image_view, nullptr);
	vkDestroyImage(device, depth_image, nullptr);
	vkFreeMemory(device, depth_image_memory, nullptr);
}
void App::createSyncObjects() {
	this->image_available_semaphores.resize(semaphore_size);
	this->render_finished_semaphores.resize(semaphore_size);
	this->in_flight_fences.resize(semaphore_size);
	this->images_in_flight.resize(this->swapchain_images.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < semaphore_size; i++) {
		if (vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->image_available_semaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->render_finished_semaphores[i]) != VK_SUCCESS ||
			vkCreateFence(this->device, &fenceInfo, nullptr, &this->in_flight_fences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}
void App::deleteSyncObjects() {
	for (size_t i = 0; i < semaphore_size; i++) {
		vkDestroySemaphore(device, render_finished_semaphores[i], nullptr);
		vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
		vkDestroyFence(device, in_flight_fences[i], nullptr);
	}
}




// ------------------------------ render pass ----------------------------------------

void App::createRenderPass() {
	VkAttachmentDescription color_attachment{};
	color_attachment.format = this->swapchain_image_format;
	color_attachment.samples = this->sample_count_falg_bits;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment{};
	depth_attachment.format = findDepthFormat(this->physical_device);
	depth_attachment.samples = this->sample_count_falg_bits;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription color_attachment_resolve{};
	color_attachment_resolve.format = this->swapchain_image_format;
	color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref{};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_attachment_resolve_ref{};
	color_attachment_resolve_ref.attachment = 2;
	color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;
	subpass.pResolveAttachments = &color_attachment_resolve_ref;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 3> attachments = { color_attachment, depth_attachment, color_attachment_resolve };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(this->device, &renderPassInfo, nullptr, &this->render_pass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

}
void App::deleteRenderPass() {
	vkDestroyRenderPass(this->device, this->render_pass, nullptr);
}

void App::createOffscreenRenderPass() {
	VkAttachmentDescription attachment_description{};
	attachment_description.format = VK_FORMAT_D16_UNORM;
	attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 0;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 0;
	subpass.pDepthStencilAttachment = &depth_reference;

	std::array<VkSubpassDependency, 2> dependencies;
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo render_pass_create_info{};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &attachment_description;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;
	render_pass_create_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
	render_pass_create_info.pDependencies = dependencies.data();

	if (vkCreateRenderPass(this->device, &render_pass_create_info, nullptr, &this->offscreen_render_pass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create offscreen render pass");
	}

}
void App::deleteOffscreenRenderPass() {
	vkDestroyRenderPass(this->device, this->offscreen_render_pass, nullptr);
}


// ------------------------------ pipeline ------------------------------------

void App::preparePipelines() {
	// shader module 作成
	auto vert_shader_code = readFile("shaders/vert.spv");
	auto frag_shader_code = readFile("shaders/frag.spv");
	VkShaderModule vert_shader_module = createShaderModule(vert_shader_code);
	VkShaderModule frag_shader_module = createShaderModule(frag_shader_code);
	// 頂点シェーダ
	VkPipelineShaderStageCreateInfo vert_shader_stage_info = vulkan::utils::getShaderStageCreateInfo(vert_shader_module, VK_SHADER_STAGE_VERTEX_BIT);
	// フラグメントシェーダ
	VkPipelineShaderStageCreateInfo frag_shader_stage_info = vulkan::utils::getShaderStageCreateInfo(frag_shader_module, VK_SHADER_STAGE_FRAGMENT_BIT);
	// パイプラインのシェーダ全部を設定
	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };
	// 頂点シェーダに渡すやつを設定する
	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	auto binding_description = Vertex::getBindingDescription();
	auto attribute_descriptions = Vertex::getAttributeDescriptions();
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
	vertex_input_info.pVertexBindingDescriptions = &binding_description;
	vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = (float)this->swapchain_extent.height;
	viewport.width = (float)this->swapchain_extent.width;
	viewport.height = -(float)this->swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = this->swapchain_extent;

	// ビューポートの情報設定
	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	// ラスタライズの情報設定
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// マルチサンプル情報設定
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = this->sample_count_falg_bits;

	// デプスステンシルの情報設定
	VkPipelineDepthStencilStateCreateInfo depth_stencil{};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = VK_TRUE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable = VK_FALSE;

	// パイプラインカラーブレンドアタッチメントの情報設定
	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	// パイプラインカラーブレンドステートの情報設定
	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	// 動的に変更される可能性のあるものを指定する
	std::vector<VkDynamicState> dynamic_state = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{};
	pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipeline_dynamic_state_create_info.pDynamicStates = dynamic_state.data();
	pipeline_dynamic_state_create_info.dynamicStateCount = dynamic_state.size();
	pipeline_dynamic_state_create_info.flags = 0;


	// パイプラインレイアウトの情報設定
	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &this->descriptor_set_layout;

	// パイプラインレイアウト生成
	if (vkCreatePipelineLayout(this->device, &pipeline_layout_info, nullptr, &this->pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// グラフィックスパイプラインの情報を設定
	VkGraphicsPipelineCreateInfo pipeline_create_info{};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.stageCount = 2;
	pipeline_create_info.pStages = shader_stages;
	pipeline_create_info.pVertexInputState = &vertex_input_info;
	pipeline_create_info.pInputAssemblyState = &input_assembly;
	pipeline_create_info.pViewportState = &viewport_state;
	pipeline_create_info.pRasterizationState = &rasterizer;
	pipeline_create_info.pMultisampleState = &multisampling;
	pipeline_create_info.pDepthStencilState = &depth_stencil;
	pipeline_create_info.pColorBlendState = &color_blending;
	pipeline_create_info.pDynamicState = &pipeline_dynamic_state_create_info;
	pipeline_create_info.layout = this->pipeline_layout;
	pipeline_create_info.renderPass = this->render_pass;
	pipeline_create_info.subpass = 0;
	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

	// グラフィックスパイプライン生成
	if (vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &this->graphics_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// 各シェーダモジュールを解放する
	vkDestroyShaderModule(this->device, frag_shader_module, nullptr);
	vkDestroyShaderModule(this->device, vert_shader_module, nullptr);



	// ----------- offscreen

	pipeline_create_info.stageCount = 1;
	std::vector<char> offscreen_vert_shader_code = readFile("shaders/offscreen_vert.spv");
	VkShaderModule offscreen_vert_shader_module = createShaderModule(offscreen_vert_shader_code);
	VkPipelineShaderStageCreateInfo offscreen_vert_shader_stage_info = vulkan::utils::getShaderStageCreateInfo(offscreen_vert_shader_module, VK_SHADER_STAGE_VERTEX_BIT);
	shader_stages[0] = vert_shader_stage_info;

	dynamic_state.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipeline_dynamic_state_create_info.pDynamicStates = dynamic_state.data();
	pipeline_dynamic_state_create_info.dynamicStateCount = dynamic_state.size();
	pipeline_dynamic_state_create_info.flags = 0;

	color_blending.attachmentCount = 0;

	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	rasterizer.depthBiasEnable = VK_TRUE;

	// 動的に変更される可能性のあるものを指定する
	std::vector<VkDynamicState> dynamic_state_enables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	pipeline_create_info.renderPass = this->offscreen_render_pass;

	if (vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &this->offscreen_pipeline)) {
		throw std::runtime_error("failed to create offscreen graphics pipeline");
	}

	vkDestroyShaderModule(this->device, vert_shader_module, nullptr);
}


// ------------------------------ shader --------------------------------------------
std::vector<char> App::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t file_size = (size_t)file.tellg();
	std::vector<char> buffer(file_size);

	file.seekg(0);
	file.read(buffer.data(), file_size);

	file.close();

	return buffer;
}
VkShaderModule App::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}


// ------------------------------ buffers --------------------------------------------
void App::loadModel() {
	std::string model_file_path = "models/sphere.obj";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_file_path.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.tex_coord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(this->vertices.size());
				this->vertices.push_back(vertex);
			}

			this->indices.push_back(uniqueVertices[vertex]);
		}
	}
}
void App::createVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(this->vertices[0]) * this->vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, this->vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(this->device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->vertex_buffer, this->vertex_buffer_memory);

	copyBuffer(stagingBuffer, this->vertex_buffer, bufferSize);

	vkDestroyBuffer(this->device, stagingBuffer, nullptr);
	vkFreeMemory(this->device, stagingBufferMemory, nullptr);

}
void App::deleteVertexBuffer() {
	vkDestroyBuffer(this->device, this->vertex_buffer, nullptr);
	vkFreeMemory(this->device, this->vertex_buffer_memory, nullptr);
}
void App::createIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(this->indices[0]) * this->indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, this->indices.data(), (size_t)bufferSize);
	vkUnmapMemory(this->device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->index_buffer, this->index_buffer_memory);

	copyBuffer(stagingBuffer, this->index_buffer, bufferSize);

	vkDestroyBuffer(this->device, stagingBuffer, nullptr);
	vkFreeMemory(this->device, stagingBufferMemory, nullptr);

}
void App::deleteIndexBuffer() {
	vkDestroyBuffer(this->device, this->index_buffer, nullptr);
	vkFreeMemory(this->device, this->index_buffer_memory, nullptr);
}
void App::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer command_buffer = this->beginSingleTimeCommands();
	VkBufferCopy copy_region{};
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, srcBuffer, dstBuffer, 1, &copy_region);
	this->endSingleTimeCommands(command_buffer);
}

void App::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	for (auto& sphere : this->spheres) {
		//// createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sphere->uniform_buffer, sphere->device_memory);
		sphere.createUniformBuffer(this->device, this->physical_device);
		sphere.createUniformBufferOffscreen(this->device, this->physical_device);
	}

}
void App::updateUniformBuffers() {
	for (auto sphere : this->spheres) {
		sphere.updateUniformBuffer(this->device, this->camera, this->swapchain_extent);
	}
}
void App::updateUniformBufferOffscreen() {
	for (auto sphere : this->spheres) {
		sphere.updateUniformBufferOffscreen(this->device, this->light, this->swapchain_extent);
	}
}


// ------------------------------ textures --------------------------------------------

void App::prepareTexture() {
	this->createTextureImage(this->texture_file_path);
	this->createTextureImageView();
	this->createTextureSampler();
}
void App::deleteTexture() {
	vkDestroySampler(this->device, this->texture_sampler, nullptr);
	vkDestroyImageView(this->device, this->texture_image_view, nullptr);
	vkDestroyImage(this->device, this->texture_image, nullptr);
	vkFreeMemory(this->device, this->texture_image_memory, nullptr);
}

void App::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(commandBuffer);
}
void App::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer);
}
void App::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(this->physical_device, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	endSingleTimeCommands(commandBuffer);
}
void App::createTextureImage(std::string texture_file_path) {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texture_file_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	this->mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(this->device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(this->device, stagingBufferMemory);

	stbi_image_free(pixels);

	createImage(texWidth, texHeight, this->mip_levels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->texture_image, this->texture_image_memory);

	transitionImageLayout(this->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, this->mip_levels);
	copyBufferToImage(stagingBuffer, this->texture_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	vkDestroyBuffer(this->device, stagingBuffer, nullptr);
	vkFreeMemory(this->device, stagingBufferMemory, nullptr);

	generateMipmaps(this->texture_image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, this->mip_levels);
}
void App::createTextureImageView() {
	this->texture_image_view = this->createImageView(this->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, this->mip_levels);
}
void App::createTextureSampler() {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(this->mip_levels);
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(this->device, &samplerInfo, nullptr, &this->texture_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}



// ------------------------------ command ----------------------------------------

void App::createCommandPool() {
	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = this->graphics_queue_index.value();
	if (vkCreateCommandPool(this->device, &pool_info, nullptr, &this->command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics command pool!");
	}
}
void App::deleteCommandPool() {
	vkDestroyCommandPool(this->device, this->command_pool, nullptr);
}
void App::createCommandBuffers() {
	this->command_buffers.resize(this->swapchain_framebuffers.size());

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = this->command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)this->command_buffers.size();

	if (vkAllocateCommandBuffers(this->device, &alloc_info, this->command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}
void App::deleteCommandBuffers() {
	vkFreeCommandBuffers(this->device, this->command_pool, static_cast<uint32_t>(this->command_buffers.size()), this->command_buffers.data());
}
void App::prepareCommand() {
	for (size_t i = 0; i < this->command_buffers.size(); i++) {
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// コマンド記録開始
		if (vkBeginCommandBuffer(this->command_buffers[i], &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		{
			// 1番目のパス : 光源から見た深度レンダリング
			// offscreen rendering

			// todo : prepare offscreen framebuffer
			VkRenderPassBeginInfo render_pass_info{};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = this->offscreen_render_pass;
			// render_pass_info.framebuffer = this->swapchain_framebuffers[i];
			render_pass_info.renderArea.offset = { 0, 0 };
			// render_pass_info.renderArea.extent = this->swapchain_extent;

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			render_pass_info.clearValueCount = 1;
			// render_pass_info.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(this->command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

			// 共通のパイプライン
			vkCmdBindPipeline(this->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphics_pipeline);

			// 共通の頂点を使用する
			VkBuffer vertexBuffers[] = { this->vertex_buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(this->command_buffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(this->command_buffers[i], this->index_buffer, 0, VK_INDEX_TYPE_UINT32);


			// デスクリプタセットのみ各オブジェクト別に割り当てる
			for (auto sphere : this->spheres) {
				sphere.bindGraphicsDescriptorSets(this->command_buffers[i], this->pipeline_layout);
				vkCmdDrawIndexed(this->command_buffers[i], static_cast<uint32_t>(this->indices.size()), 1, 0, 0, 0);
			}

			vkCmdEndRenderPass(this->command_buffers[i]);
		}


		{
			// 2番目のパス : シーンレンダリング
			// 普通に画面に出力するやつ
			VkRenderPassBeginInfo render_pass_info{};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = this->render_pass;
			render_pass_info.framebuffer = this->swapchain_framebuffers[i];
			render_pass_info.renderArea.offset = { 0, 0 };
			render_pass_info.renderArea.extent = this->swapchain_extent;

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
			render_pass_info.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(this->command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

			// 共通のパイプライン
			vkCmdBindPipeline(this->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphics_pipeline);

			// 共通の頂点を使用する
			VkBuffer vertex_buffers[] = { this->vertex_buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(this->command_buffers[i], 0, 1, vertex_buffers, offsets);
			vkCmdBindIndexBuffer(this->command_buffers[i], this->index_buffer, 0, VK_INDEX_TYPE_UINT32);


			// デスクリプタセットのみ各オブジェクト別に割り当てる
			for (auto sphere : this->spheres) {
				sphere.bindGraphicsDescriptorSets(this->command_buffers[i], this->pipeline_layout);
				vkCmdDrawIndexed(this->command_buffers[i], static_cast<uint32_t>(this->indices.size()), 1, 0, 0, 0);
			}

			vkCmdEndRenderPass(this->command_buffers[i]);
		}

		// コマンド記録終了
		if (vkEndCommandBuffer(this->command_buffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

VkCommandBuffer App::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = this->command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(this->device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}
void App::endSingleTimeCommands(VkCommandBuffer command_buffer) {
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffer;

	vkQueueSubmit(this->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(this->graphics_queue);

	vkFreeCommandBuffers(this->device, this->command_pool, 1, &command_buffer);
}



// ------------------------------ descriptor ----------------------------------------
void App::prepareDescriptor() {
	this->createDescriptorPool();
	this->createDescriptorSetLayout();
	this->createDescriptorSets();
}

void App::deleteDescriptor() {
	vkDestroyDescriptorSetLayout(this->device, this->descriptor_set_layout, nullptr);
}
void App::createDescriptorPool() {
	// 描画する球体の数だけユニフォームバッファとテクスチャサンプラーが作成される可能性がある
	std::array<VkDescriptorPoolSize, 2> pool_sizes{};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = static_cast<uint32_t>(this->sphere_count);
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = static_cast<uint32_t>(this->sphere_count);

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_info.pPoolSizes = pool_sizes.data();
	pool_info.maxSets = static_cast<uint32_t>(this->swapchain_images.size() * this->sphere_count);

	if (vkCreateDescriptorPool(this->device, &pool_info, nullptr, &this->descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}
void App::createDescriptorSetLayout() {
	std::array<VkDescriptorSetLayoutBinding, 2> set_layout_bindings{};

	// 定数バッファのバインド
	set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	set_layout_bindings[0].binding = 0;
	set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	set_layout_bindings[0].descriptorCount = 1;

	// テクスチャサンプラーのバインド
	set_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	set_layout_bindings[1].binding = 1;
	set_layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	set_layout_bindings[1].descriptorCount = 1;

	VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info{};
	descriptor_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout_create_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
	descriptor_layout_create_info.pBindings = set_layout_bindings.data();

	if (vkCreateDescriptorSetLayout(this->device, &descriptor_layout_create_info, nullptr, &this->descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout");
	}
}
void App::createDescriptorSets() {
	for (auto& sphere : this->spheres) {
		VkDescriptorSetAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocate_info.descriptorPool = this->descriptor_pool;
		allocate_info.descriptorSetCount = 1;
		allocate_info.pSetLayouts = &this->descriptor_set_layout;
		sphere.allocateGraphicsDescriptorSets(this->device, allocate_info);

		// テクスチャも共通のものを使う、何ならここではテクスチャいらないかも
		VkDescriptorBufferInfo buffer_info{};
		buffer_info.buffer = sphere.uniform_buffer;
		buffer_info.offset = 0;
		buffer_info.range = sizeof(UniformBufferObject);
		VkDescriptorImageInfo image_info{};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = this->texture_image_view;
		image_info.sampler = this->texture_sampler;

		sphere.writeGraphicsDescriptorSets(this->device, &buffer_info, &image_info);
	}
}




