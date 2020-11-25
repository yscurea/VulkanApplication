#pragma

#include <vulkan/vulkan.h>

#include "./Window.h"
#include "./Surface.h"
#include "./Debug.h"
#include "./Device.h"

namespace vulkan::base {

	// vulkan���g�����A�v���̃x�[�X
	class AppBase {
		Window window;
		VkInstance instance;
		Debug debug;
		Surface surface;
		Device device;

		VkCommandPool command_pool;
		void createCommandPool();
	public:
		void init();

		virtual void renderLoop();

		virtual void cleanup();
	};

}