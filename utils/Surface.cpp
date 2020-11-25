#include "Surface.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

void Surface::createSurface() {
	if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface");
	}
}

void Surface::deleteSurface() {
	vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
}
