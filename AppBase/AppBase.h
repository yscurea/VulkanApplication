#pragma

#include <vulkan/vulkan.h>

#include "../utils/Device.h"

// vulkan���g�����A�v���̃x�[�X
class AppBase {
	VkInstance instance;
	Device device;
public:
	void init();

	virtual void renderLoop();

	virtual void cleanup();
};