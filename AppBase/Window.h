#pragma once

// �v���b�g�t�H�[�����Ƃ̏����R���p�C���ɂȂ邩���H

#include <GLFW/glfw3.h>

class Window {
	GLFWwindow* winodw;
public:
	void createWindow();
	void deleteWindow();
};