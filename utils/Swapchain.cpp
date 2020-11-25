#include "Swapchain.h"

void Swapchain::createSwapchain() {
	SwapchainSupportDetails swapchain_support = querySwapChainSupport(this->physical_device);

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

void Swapchain::deleteSwapchain() {
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

void Swapchain::createSwapchainImageViews() {
	this->swapchain_image_views.resize(this->swapchain_images.size());

	for (uint32_t i = 0; i < this->swapchain_images.size(); i++) {
		this->swapchain_image_views[i] = createImageView(this->swapchain_images[i], this->swapchain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void Swapchain::deleteSwapchainImageViews() {
	for (auto image_view : this->swapchain_image_views) {
		vkDestroyImageView(this->device, image_view, nullptr);
	}
}
