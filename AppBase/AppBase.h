#pragma

#include <vulkan/vulkan.h>

#include "./Window.h"
#include "./Surface.h"
#include "./Debug.h"
#include "./Device.h"

namespace vulkan::base {

	// vulkanを使ったアプリのベース
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