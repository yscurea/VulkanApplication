#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>


#include <unordered_map>

void Model::load(std::string file_path) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_path.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> unique_vertices{};

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

			if (unique_vertices.count(vertex) == 0) {
				unique_vertices[vertex] = static_cast<uint32_t>(this->vertices.size());
				this->vertices.push_back(vertex);
			}

			this->indices.push_back(unique_vertices[vertex]);
		}
	}
}

void Model::bindBuffers(VkCommandBuffer command_buffer) {
	VkBuffer vertexBuffers[] = { this->vertex_buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(command_buffer, this->index_buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Model::drawIndex(VkCommandBuffer command_buffer) {
	vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(this->indices.size()), 1, 0, 0, 0);
}