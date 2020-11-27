#include "AppBase.h"


namespace vulkan::base {

	void AppBase::connect(Window* window, Instance* instance, Debug* debug, Surface* surface, Device* device, Swapchain* swapchain) {

	}

	void AppBase::init() {
		this->connect(&this->window, &this->instance, &this->debug, &this->surface, &this->device, &this->swapchain);
		this->window.createWindow();
		this->instance.createInstance();
		this->debug.setupDebugMessenger();
		this->surface.createSurface();
		this->device.createLogicalDevice();
		this->swapchain.createSwapchain();
	}

	void AppBase::renderLoop() {

	}

	void AppBase::cleanup() {

	}

}