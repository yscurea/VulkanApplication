#pragma once

#include <glm/glm.hpp>


struct Light
{
	float fov = glm::radians(45.0f);
	float near_clip = 0.1f;
	float far_clip = 200.0f;
	glm::vec3 position = glm::vec3(0.0f, 40.0f, 0.0f);
};

