#include "Object.h"


void Object::draw(VkCommandBuffer command_buffer) {
	this->model->drawIndex(command_buffer);
}

void Object::setModel(Model* model) {
	this->model = model;
}
