#include "App.h"

#include <array>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <set>
#include <array>
#include <algorithm>

void App::initVulkan() {
	this->initWindow();
	this->createInstance();
#ifdef _DEBUG
	// this->setupDebug();
#endif
	this->createSurface();
	this->selectPhysicalDevice();
	this->createLogicalDevice();
	this->createSwapchain();
	this->createRenderPass();
}
void App::run() {
	this->prepare();
	while (!glfwWindowShouldClose(this->window)) {
		glfwPollEvents();
		this->render();
	}
	vkDeviceWaitIdle(this->device);
	this->cleanup();
}
void App::render() {
	vkWaitForFences(this->device, 1, &this->in_flight_fences[this->current_frame], VK_TRUE, UINT64_MAX);

	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(this->device, this->swapchain, UINT64_MAX, this->image_available_semaphores[this->current_frame], VK_NULL_HANDLE, &image_index);

	this->updateUniformBuffers();

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
void App::prepare() {
	// �C���X�^���X�A�f�o�C�X�A�X���b�v�`�F�C���A�T�[�t�F�X�A�E�B���h�E
	this->initVulkan();

	this->createDescriptorPool();
	this->createDescriptorSetLayout();
	this->createDescriptorSets();
	// �R�}���h�v�[���쐬
	this->createCommandPool();
	// �R�}���h�o�b�t�@�쐬
	this->createCommandBuffers();
	// �R�}���h�L�^
	this->prepareCommand();

	this->loadModel();
}
void App::cleanup() {

	glfwDestroyWindow(this->window);
	glfwTerminate();
	this->deleteSwapchain();
	// this->deleteLogicalDevice();
	this->deleteSurface();
	// debug
	this->deleteInstance();
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

}




// ------------------------------ instance ----------------------------------------
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

void App::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}



// ------------------------------ surface ----------------------------------------
void App::createSurface() {
	if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface");
	}
}
void App::deleteSurface() {

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

}
void App::deleteSwapchain() {
	vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
}
void App::createSwapchainImages() {
}
void App::deleteSwapchainImages() {
}
void App::createSwapchainImageViews() {
	for (auto image_view : this->swapchain_image_views) {
		vkDestroyImageView(this->device, image_view, nullptr);
	}
}
void App::deleteSwapchainImageViews() {

}
void App::createSwapchainFrameBuffers() {

}
void App::deleteSwapchainFrameBuffers() {
	for (auto framebuffer : this->swapchain_framebuffers) {
		vkDestroyFramebuffer(this->device, framebuffer, nullptr);
	}
}
void App::createColorResources() {

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


// ------------------------------ graphics pipeline ------------------------------------
void App::createGraphcisPipeline() {
	auto vert_shader_code = readFile("shaders/vert.spv");
	auto frag_shader_code = readFile("shaders/frag.spv");

	VkShaderModule vert_shader_module = createShaderModule(vert_shader_code);
	VkShaderModule frag_shader_module = createShaderModule(frag_shader_code);

	// ���_�V�F�[�_
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vert_shader_module;
	vertShaderStageInfo.pName = "main";

	// �t���O�����g�V�F�[�_
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = frag_shader_module;
	fragShaderStageInfo.pName = "main";

	// �p�C�v���C���̃V�F�[�_�S����ݒ�
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// ���_�V�F�[�_�ɓn�����ݒ肷��
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

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

	// �r���[�|�[�g�̏��ݒ�
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// ���X�^���C�Y�̏��ݒ�
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// �}���`�T���v�����ݒ�
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = this->sample_count_falg_bits;

	// �f�v�X�X�e���V���̏��ݒ�
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	// �p�C�v���C���J���[�u�����h�A�^�b�`�����g�̏��ݒ�
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	// �p�C�v���C���J���[�u�����h�X�e�[�g�̏��ݒ�
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// �p�C�v���C�����C�A�E�g�̏��ݒ�
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &this->descriptor_set_layout;

	// �p�C�v���C�����C�A�E�g����
	if (vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// �O���t�B�b�N�X�p�C�v���C���̏���ݒ�
	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shaderStages;
	pipeline_info.pVertexInputState = &vertexInputInfo;
	pipeline_info.pInputAssemblyState = &inputAssembly;
	pipeline_info.pViewportState = &viewportState;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depthStencil;
	pipeline_info.pColorBlendState = &colorBlending;
	pipeline_info.layout = this->pipeline_layout;
	pipeline_info.renderPass = this->render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	// �O���t�B�b�N�X�p�C�v���C������
	if (vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &this->graphics_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// �e�V�F�[�_���W���[�����������
	vkDestroyShaderModule(this->device, frag_shader_module, nullptr);
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

		// �R�}���h�L�^�J�n
		if (vkBeginCommandBuffer(this->command_buffers[i], &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
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

		vkCmdBindPipeline(this->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphics_pipeline);

		// ���ʂ̃��f�����g�p����
		this->unique_model.bindBuffers(this->command_buffers[i]);

		// �f�X�N���v�^�Z�b�g�̂݊e�I�u�W�F�N�g�ʂɊ��蓖�Ă�
		for (auto sphere : this->spheres) {
			sphere->bindDescriptorSets(this->command_buffers[i], this->pipeline_layout);
			sphere->draw(this->command_buffers[i]);
		}

		vkCmdEndRenderPass(this->command_buffers[i]);

		// �R�}���h�L�^�I��
		if (vkEndCommandBuffer(this->command_buffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

}



// ------------------------------ descriptor ----------------------------------------
void App::createDescriptorPool() {
	// �`�悷�鋅�̂̐��������j�t�H�[���o�b�t�@�ƃe�N�X�`���T���v���[���쐬�����\��������
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

	// �萔�o�b�t�@�̃o�C���h
	set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	set_layout_bindings[0].binding = 0;
	set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	set_layout_bindings[0].descriptorCount = 1;

	// �e�N�X�`���T���v���[�̃o�C���h
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
		sphere->allocateDescriptorSets(this->device, allocate_info);

		sphere->writeDescriptorSets(this->device);
	}
}





void App::loadModel() {
	std::string model_file_path = "models/sphere.obj";
	this->unique_model.load(model_file_path);
	for (auto sphere : this->spheres) {
		sphere->setModel(&this->unique_model);
	}
}
void App::updateUniformBuffers() {
	for (auto sphere : this->spheres) {
		sphere->updateUniformBuffer();
	}
}


