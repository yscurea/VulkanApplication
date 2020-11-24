#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

#include "./Vertex.h"
#include "./Texture.h"

class Model {
public:
	void load(std::string file_path);
	void bindBuffers(VkCommandBuffer command_buffer);
	void drawIndex(VkCommandBuffer command_buffer);
private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;
	// Texture texture;
};