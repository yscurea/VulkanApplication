#pragma

#include <vulkan/vulkan.h>

#include "./Window.h"
#include "./Instance.h"
#include "./Surface.h"
#include "./Debug.h"
#include "./Device.h"
#include "./Swapchain.h"

namespace vulkan::base {

	// vulkanを使ったアプリのベース
	class AppBase {
		VkInstance instance;
		Surface surface;
		Device device;
		Swapchain swapchain;

		VkCommandPool command_pool;
	public:
		void init();

		virtual void cleanup();
	};

}