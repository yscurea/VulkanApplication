#pragma once

#include <optional>

struct DeviceQueueIndices {
	std::optional<uint32_t> graphics_queue_index;
	std::optional<uint32_t> present_queue_index;
	bool isComplete();
};
