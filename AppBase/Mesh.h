#pragma once

#include <vector>

#include "Vertex.h"
#include "Buffer.h"

class Mesh {
private:
	std::vector<Vertex> vertices;
	Buffer vertex;
	std::vector<uint32_t> indices;
	Buffer index;

	// VkBuffer vertex_buffer;
	// VkDeviceMemory vertex_buffer_memory;
	// VkBuffer index_buffer;
	// VkDeviceMemory index_buffer_memory;

public:
	void createVertexBuffer();
	void deleteVertexBuffer();
	void createIndexBuffer();
	void deleteIndexBuffer();
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};