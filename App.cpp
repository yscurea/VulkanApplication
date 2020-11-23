#include "App.h"

#include <array>
#include <stdexcept>

void App::run() {
	this->prepare();
	while (!glfwWindowShouldClose(this->window)) {
		glfwPollEvents();
		this->render();
	}
	vkDeviceWaitIdle(this->device);
	this->cleanup();
}

void App::cleanup() {
	AppBase::cleanup();
}
void App::prepare() {
	AppBase::initVulkan();
}

void App::loadModel() {
	std::string model_file_path = "models/sphere.obj";
	this->unique_model.load(model_file_path);
	for (auto sphere : this->spheres) {
		sphere->setModel(&unique_model);
	}
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
		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = this->descriptor_pool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &this->descriptor_set_layout;
		if (vkAllocateDescriptorSets(this->device, &allocateInfo, sphere->getDescriptorSet()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets");
		}

		std::array<VkWriteDescriptorSet, 2> write_descriptor_sets{};

		// Binding 0: RenderingObject uniform buffer
		write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_sets[0].dstSet = *sphere->getDescriptorSet();
		write_descriptor_sets[0].dstBinding = 0;
		write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_descriptor_sets[0].pBufferInfo = sphere->getUniformBuffer()->getDescriptorBufferInfo();
		write_descriptor_sets[0].descriptorCount = 1;

		// Binding 1: RenderingObject texture
		write_descriptor_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_sets[1].dstSet = *sphere->getDescriptorSet();
		write_descriptor_sets[1].dstBinding = 1;
		write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_descriptor_sets[1].pImageInfo = sphere->getTexture()->getDescriptorImageInfo();
		write_descriptor_sets[1].descriptorCount = 1;

		vkUpdateDescriptorSets(this->device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
	}
}

void updateUniformBuffers() {

}

void App::prepareCommand() {
	this->command_buffers.resize(this->swapchain_framebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)this->command_buffers.size();

	if (vkAllocateCommandBuffers(this->device, &allocInfo, this->command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < this->command_buffers.size(); i++) {
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// コマンド記録開始
		if (vkBeginCommandBuffer(this->command_buffers[i], &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = this->render_pass;
		renderPassInfo.framebuffer = this->swapchain_framebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = this->swapchain_extent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(this->command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(this->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphics_pipeline);

		// 共通のモデルを使用する
		unique_model.bindBuffers(this->command_buffers[i]);

		// デスクリプタセットのみ各オブジェクト別に割り当てる
		for (auto sphere : this->spheres) {
			vkCmdBindDescriptorSets(this->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline_layout, 0, 1, &this->descriptorSets[i], 0, nullptr);

			sphere->draw(this->command_buffers[i]);
			// vkCmdDrawIndexed(this->command_buffers[i], static_cast<uint32_t>(this->indices.size()), 1, 0, 0, 0);
		}

		vkCmdEndRenderPass(this->command_buffers[i]);

		// コマンド記録終了
		if (vkEndCommandBuffer(this->command_buffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

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
