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
		void connect(Window* window, Instance* instance, Debug* debug, Surface* surface, Device* device, Swapchain* swapchain);
		Window window;
		Instance instance;
		Debug debug;
		Surface surface;
		Device device;
		Swapchain swapchain;

		VkCommandPool command_pool;
	public:
		void init();

		virtual void cleanup();
	};

}