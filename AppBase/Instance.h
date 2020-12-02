#pragma once

#include <vulkan/vulkan.h>

#include "./Debug.h"

namespace vulkan::base {

	class Instance {
		VkInstance instance;
#ifdef _DEBUG
		Debug debug;
#endif
	public:
		Instance();
		~Instance();
		void createInstance();
		void deleteInstance();
#ifdef _DEBUG
		void setupDebug();
		void deleteDebug();
#endif
	};

}