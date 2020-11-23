#include "AppBase.h"

#include <stdexcept>
#include <set>

void AppBase::initVulkan() {
	this->createWindow();
	this->createInstance();
	// debug
	this->createSurface();
	this->createLogicalDevice();
	this->createSwapchain();
}
void AppBase::cleanup() {

	glfwDestroyWindow(this->window);
	glfwTerminate();
	this->deleteSwapchain();
	// this->deleteLogicalDevice();
	this->deleteSurface();
	// debug
	this->deleteInstance();
}





// ------------------------------ window ----------------------------------------
// windowçÏê¨
void AppBase::createWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	this->window = glfwCreateWindow(window_width, window_height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(this->window, this);
	glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);
}
// windowÇ™ÉäÉTÉCÉYÇ≥ÇÍÇΩÇ∆Ç´ÇÃcallbackä÷êî
void AppBase::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<AppBase*>(glfwGetWindowUserPointer(window));
	app->framebuffer_resized = true;
}
void AppBase::deleteWindow() {
	glfwDestroyWindow(this->window);
	glfwTerminate();
}
// ------------------------------ instance ----------------------------------------
void AppBase::createInstance() {
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Sample";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	std::vector<const char*> extensions;
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> tmp_extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		extensions = tmp_extensions;
	}

	create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();

	create_info.enabledLayerCount = 0;
	create_info.pNext = nullptr;

	if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}
void AppBase::deleteInstance() {
	vkDestroyInstance(this->instance, nullptr);
}

// ------------------------------ surface ----------------------------------------
void AppBase::createSurface() {
	if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface");
	}
}
void AppBase::deleteSurface() {

}

// ------------------------------ device ----------------------------------------
DeviceQueueIndices AppBase::findDeviceQueue(VkPhysicalDevice physical_device) {
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
bool AppBase::checkDeviceExtensionSupport(VkPhysicalDevice physical_device) {
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
SwapChainSupportDetails AppBase::querySwapChainSupport(VkPhysicalDevice device) {
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
bool AppBase::isDeviceSuitable(VkPhysicalDevice physical_device) {
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
void AppBase::selectPhysicalDevice() {
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
void AppBase::createLogicalDevice() {
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
	createInfo.enabledLayerCount = 0;
#endif

	if (vkCreateDevice(this->physical_device, &create_info, nullptr, &this->device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(this->device, this->graphics_queue_index.value(), 0, &this->graphics_queue);
	vkGetDeviceQueue(this->device, this->present_queue_index.value(), 0, &this->present_queue);
}
