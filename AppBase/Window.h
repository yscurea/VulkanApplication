#pragma once

// プラットフォームごとの条件コンパイルになるかも？

#include <GLFW/glfw3.h>

class Window {
	GLFWwindow* winodw;
public:
	void createWindow();
	void deleteWindow();
};