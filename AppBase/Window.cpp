#include "./Window.h"


void Window::createWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	this->window = glfwCreateWindow(this->window_width, this->window_height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(this->window, this);
	glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);
}

void Window::deleteWindow() {
	glfwDestroyWindow(this->window);
	glfwTerminate();
}