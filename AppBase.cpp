#include "AppBase.h"

#include <stdexcept>
#include <set>
#include <array>

void AppBase::initVulkan() {
	this->createWindow();
	this->createInstance();
	// debug
	this->createSurface();
	this->selectPhysicalDevice();
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

// ------------------------------ debug ----------------------------------------
void AppBase::setupDebug() {

}
void AppBase::deleteDebug() {

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



// ------------------------------ swapchain ----------------------------------------
void AppBase::createSwapchain() {
	SwapChainSupportDetails swapchain_support = querySwapChainSupport(this->physical_device);

	VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swapchain_support.formats);
	VkPresentModeKHR present_mode = chooseSwapPresentMode(swapchain_support.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapchain_support.capabilities);

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
void AppBase::deleteSwapchain() {
	vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
}
void AppBase::createSwapchainImages() {

}
void AppBase::deleteSwapchainImages() {
}
void AppBase::createSwapchainImageViews() {
	for (auto image_view : this->swapchain_image_views) {
		vkDestroyImageView(this->device, image_view, nullptr);
	}
}
void AppBase::createSwapchainFrameBuffers() {

}
void AppBase::createColorResources() {

}
void AppBase::createDepthResources() {
	VkFormat depthFormat = this->findDepthFormat();

	this->createImage(this->swapchain_extent.width, this->swapchain_extent.height, 1, this->sample_count_falg_bits, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->depth_image, this->depth_image_memory);
	this->depth_image_view = this->createImageView(this->depth_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}
void AppBase::deleteDepthResources() {

}
void AppBase::createSyncObjects() {

}





// ------------------------------ render pass ----------------------------------------
void AppBase::createRenderPass() {
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
	depth_attachment.format = this->findDepthFormat();
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
void AppBase::deleteRenderPass() {
	vkDestroyRenderPass(this->device, this->render_pass, nullptr);
}




// ------------------------------ command ----------------------------------------
void AppBase::createCommandPool() {
	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = this->graphics_queue_index.value();
	if (vkCreateCommandPool(this->device, &pool_info, nullptr, &this->command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics command pool!");
	}
}
void AppBase::deleteCommandPool() {

}
void AppBase::createCommandBuffers() {
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
void AppBase::deleteCommandBuffers() {

}








// ------------------------------ utils ----------------------------------------
VkBuffer createBuffer() {

}
VkImage createImage() {

}
VkImageView createImageView() {

}
