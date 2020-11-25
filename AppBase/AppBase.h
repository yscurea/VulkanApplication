#pragma

#include <vulkan/vulkan.h>

#include "../utils/Device.h"

// vulkanを使ったアプリのベース
class AppBase {
	VkInstance instance;
	Device device;
public:
	void init();

	virtual void renderLoop();

	virtual void cleanup();
};