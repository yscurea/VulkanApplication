#pragma once

// �v���b�g�t�H�[�����Ƃ̏����R���p�C���ɂȂ邩���H

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