#pragma once

// プラットフォームごとの条件コンパイルになるかも？

#include <GLFW/glfw3.h>

class Window {
	GLFWwindow* window;
	uint32_t window_height;
	uint32_t window_width;
	bool is_resize = false;
public:
	bool isResize();
	void createWindow();
	void deleteWindow();
};