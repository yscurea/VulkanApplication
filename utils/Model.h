#pragma once

class Model {
public:
	void load();
private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};