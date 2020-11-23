#include "App.h"

#include <array>
#include <stdexcept>

void App::prepare() {
	AppBase::initVulkan();
}

void App::loadModel() {

}

void App::createDescriptorPool() {
	std::array<VkDescriptorPoolSize, 2> pool_sizes{};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = static_cast<uint32_t>(this->swapchain_images.size());
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = static_cast<uint32_t>(this->swapchain_images.size());

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_info.pPoolSizes = pool_sizes.data();
	pool_info.maxSets = static_cast<uint32_t>(swapchain_images.size());

	if (vkCreateDescriptorPool(this->device, &pool_info, nullptr, &this->descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}
void App::createDescriptorSetLayout() {
	std::array<VkDescriptorSetLayoutBinding, 2> set_layout_bindings{};

	// 定数バッファに使う
	set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	set_layout_bindings[0].binding = 0;
	set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	set_layout_bindings[0].descriptorCount = 1;

	// テクスチャサンプラーに使う
	set_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	set_layout_bindings[1].binding = 1;
	set_layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	set_layout_bindings[1].descriptorCount = 1;

	VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
	descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCI.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
	descriptorLayoutCI.pBindings = set_layout_bindings.data();

	if (vkCreateDescriptorSetLayout(this->device, &descriptorLayoutCI, nullptr, &this->descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout");
	}
}
void App::createDescriptorSets() {
	for (auto& sphere : this->spheres) {
		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = this->descriptor_pool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &this->descriptor_set_layout;
		if (vkAllocateDescriptorSets(this->device, &allocateInfo, &sphere.descriptor_set) != VK_SUCCESS) {

		}

		std::array<VkWriteDescriptorSet, 2> write_descriptor_sets{};

		// Binding 0: RenderingObject uniform buffer
		write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_sets[0].dstSet = sphere.descriptor_set;
		write_descriptor_sets[0].dstBinding = 0;
		write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_descriptor_sets[0].pBufferInfo = &sphere.uniform_buffer.descriptor_buffer_info;
		write_descriptor_sets[0].descriptorCount = 1;

		// Binding 1: RenderingObject texture
		write_descriptor_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_sets[1].dstSet = sphere.descriptor_set;
		write_descriptor_sets[1].dstBinding = 1;
		write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_descriptor_sets[1].pImageInfo = &sphere.texture.descriptor_image_info;
		write_descriptor_sets[1].descriptorCount = 1;

		vkUpdateDescriptorSets(this->device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
	}
}

void updateUniformBuffers() {

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

	VkResult result = vkQueuePresentKHR(this->present_queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->framebuffer_resized) {
		this->framebuffer_resized = false;
		this->recreateSwapchain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
}

void App::run() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	this->window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
	while (!glfwWindowShouldClose(this->window)) {
		glfwPollEvents();
		this->render();
	}
	glfwDestroyWindow(this->window);
	glfwTerminate();
}
