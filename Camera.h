#pragma once

#include <glm/glm.hpp>

struct Camera
{
	float fov = glm::radians(45.0f);
	float near_clip = 0.1f;
	float far_clip = 200.0f;
	glm::vec3 position = { -10.0f,-10.0f,-10.0f };
	glm::vec3 target_position = { 0.0f,0.0f,0.0f };
	glm::vec3 rotation = { 0.0f,0.0f,0.0f };
	glm::mat4 projection;
};

