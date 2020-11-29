#include "Vertex.h"

VkVertexInputBindingDescription Vertex::getBindingDescription() {
	VkVertexInputBindingDescription binding_description{};
	binding_description.binding = 0;
	binding_description.stride = sizeof(Vertex);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return binding_description;
}


std::array<VkVertexInputAttributeDescription, 4> Vertex::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions{};

	attribute_descriptions[0].binding = 0;
	attribute_descriptions[0].location = 0;
	attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[0].offset = offsetof(Vertex, pos);

	attribute_descriptions[1].binding = 0;
	attribute_descriptions[1].location = 1;
	attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[1].offset = offsetof(Vertex, color);

	attribute_descriptions[2].binding = 0;
	attribute_descriptions[2].location = 2;
	attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

	attribute_descriptions[3].binding = 0;
	attribute_descriptions[3].location = 3;
	attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[3].offset = offsetof(Vertex, normal);

	return attribute_descriptions;
}



bool Vertex::operator==(const Vertex& other) const {
	return (pos == other.pos && color == other.color && tex_coord == other.tex_coord);
}

