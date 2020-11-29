#pragma once

#include <glm/glm.hpp>

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 depth_light_model;
	glm::mat4 depth_light_view;
	glm::mat4 depth_light_projection;
	glm::vec3 light_position;
};