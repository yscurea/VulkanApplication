#include "App.h"

// todo: クラス設計の見直し

int main() {
	uint32_t sphere_count = 300;
	App* app = new App(sphere_count);
	app->run();
	return 0;
}