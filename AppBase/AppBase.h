#pragma

#include <vulkan/vulkan.h>

#include "../utils/Device.h"

namespace vulkan::base {

	// vulkanを使ったアプリのベース
	class AppBase {
		VkInstance instance;
		Device device;

		VkCommandPool command_pool;
		void createCommandPool();
	public:
		void init();

		virtual void renderLoop();

		virtual void cleanup();
	};

}