#pragma once

#include <optional>

struct DeviceQueueIndices {
	std::optional<uint32_t> graphics_queue_index;
	std::optional<uint32_t> present_queue_index;
	bool isComplete() {
		return this->graphics_queue_index.has_value() && this->present_queue_index.has_value();
	}
};
