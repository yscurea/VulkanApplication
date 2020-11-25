#include "DeviceQueueIndices.h"


bool DeviceQueueIndices::isComplete() {
	return this->graphics_queue_index.has_value() && this->present_queue_index.has_value();
}