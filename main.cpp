#include "App.h"

// todo: �N���X�݌v�̌�����

int main() {
	uint32_t sphere_count = 3000;
	App* app = new App(sphere_count);
	app->run();
	return 0;
}