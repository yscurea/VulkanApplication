#include "utils.h"


namespace vulkan::utils {
	void checkError(VkResult result) {
		switch (result)
		{
		case VK_SUCCESS:
			break;
		case VK_NOT_READY:
			throw std::runtime_error("VK_NOT_READY");
			break;
		case VK_TIMEOUT:
			throw std::runtime_error("VK_TIMEOUT");
			break;
		case VK_EVENT_SET:
			throw std::runtime_error("VK_EVENT_SET");
			break;
		case VK_EVENT_RESET:
			throw std::runtime_error("VK_EVENT_RESET");
			break;
		case VK_INCOMPLETE:
			throw std::runtime_error("VK_INCOMPLETE");
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			throw std::runtime_error("VK_ERROR_OUT_OF_HOST_MEMORY");
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			throw std::runtime_error("VK_ERROR_OUT_OF_DEVICE_MEMORY");
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			throw std::runtime_error("VK_ERROR_INITIALIZATION_FAILED");
			break;
		case VK_ERROR_DEVICE_LOST:
			throw std::runtime_error("VK_ERROR_DEVICE_LOST");
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			throw std::runtime_error("VK_ERROR_MEMORY_MAP_FAILED");
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			throw std::runtime_error("VK_ERROR_LAYER_NOT_PRESENT");
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			throw std::runtime_error("VK_ERROR_EXTENSION_NOT_PRESENT");
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			throw std::runtime_error("VK_ERROR_FEATURE_NOT_PRESENT");
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			throw std::runtime_error("VK_ERROR_INCOMPATIBLE_DRIVER");
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			throw std::runtime_error("VK_ERROR_TOO_MANY_OBJECTS");
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			throw std::runtime_error("VK_ERROR_FORMAT_NOT_SUPPORTED");
			break;
		case VK_ERROR_FRAGMENTED_POOL:
			throw std::runtime_error("VK_ERROR_FRAGMENTED_POOL");
			break;
		case VK_ERROR_UNKNOWN:
			throw std::runtime_error("VK_ERROR_UNKNOWN");
			break;
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			throw std::runtime_error("VK_ERROR_OUT_OF_POOL_MEMORY");
			break;
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			throw std::runtime_error("VK_ERROR_INVALID_EXTERNAL_HANDLE");
			break;
		case VK_ERROR_FRAGMENTATION:
			throw std::runtime_error("VK_ERROR_FRAGMENTATION");
			break;
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			throw std::runtime_error("VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS");
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			throw std::runtime_error("VK_ERROR_SURFACE_LOST_KHR");
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			throw std::runtime_error("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR");
			break;
		case VK_SUBOPTIMAL_KHR:
			throw std::runtime_error("VK_SUBOPTIMAL_KHR");
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			throw std::runtime_error("VK_ERROR_OUT_OF_DATE_KHR");
			break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			throw std::runtime_error("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR");
			break;
		case VK_ERROR_VALIDATION_FAILED_EXT:
			throw std::runtime_error("VK_ERROR_VALIDATION_FAILED_EXT");
			break;
		case VK_ERROR_INVALID_SHADER_NV:
			throw std::runtime_error("VK_ERROR_INVALID_SHADER_NV");
			break;
		case VK_ERROR_INCOMPATIBLE_VERSION_KHR:
			throw std::runtime_error("VK_ERROR_INCOMPATIBLE_VERSION_KHR");
			break;
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			throw std::runtime_error("VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT");
			break;
		case VK_ERROR_NOT_PERMITTED_EXT:
			throw std::runtime_error("VK_ERROR_NOT_PERMITTED_EXT");
			break;
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			throw std::runtime_error("VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT");
			break;
		case VK_THREAD_IDLE_KHR:
			throw std::runtime_error("VK_THREAD_IDLE_KHR");
			break;
		case VK_THREAD_DONE_KHR:
			throw std::runtime_error("VK_THREAD_DONE_KHR");
			break;
		case VK_OPERATION_DEFERRED_KHR:
			throw std::runtime_error("VK_OPERATION_DEFERRED_KHR");
			break;
		case VK_OPERATION_NOT_DEFERRED_KHR:
			throw std::runtime_error("VK_OPERATION_NOT_DEFERRED_KHR");
			break;
		case VK_PIPELINE_COMPILE_REQUIRED_EXT:
			throw std::runtime_error("VK_PIPELINE_COMPILE_REQUIRED_EXT");
			break;
			/*
			case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
				throw std::runtime_error("VK_ERROR_OUT_OF_POOL_MEMORY_KHR");
				break;
			case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:
				throw std::runtime_error("VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR");
				break;
			case VK_ERROR_FRAGMENTATION_EXT:
				throw std::runtime_error("VK_ERROR_FRAGMENTATION_EXT");
				break;
			case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
				throw std::runtime_error("VK_ERROR_INVALID_DEVICE_ADDRESS_EXT");
				break;
			case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR:
				throw std::runtime_error("VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR");
				break;
			case VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT:
				throw std::runtime_error("VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT");
				break;
			*/
		case VK_RESULT_MAX_ENUM:
			throw std::runtime_error("VK_RESULT_MAX_ENUM");
			break;
		default:
			break;
		}
	}

	namespace initializer {
		VkApplicationInfo getApplicationInfo(const char* application_name) {
			VkApplicationInfo app_info{};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = application_name;
			app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.pEngineName = "No Engine";
			app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.apiVersion = VK_API_VERSION_1_2;
			return app_info;
		}
		VkInstanceCreateInfo getInstanceCreateInfo(VkApplicationInfo* application_info, std::vector<const char*> extensions) {
			VkInstanceCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.pApplicationInfo = &application_info;
			create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			create_info.ppEnabledExtensionNames = extensions.data();
			return create_info;
		}

		VkSwapchainCreateInfoKHR getSwapchainCreateInfoKHR() {

		}

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr) {
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else {
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}
	}
}