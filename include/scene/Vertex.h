#pragma once
#include <glm/glm.hpp>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class Vertex
{
public:

	Vertex();
	Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 color, glm::vec2 texture_coords);

	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texture_coords;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && normal == other.normal && texture_coords == other.texture_coords;
	}

};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			size_t h1 = hash<glm::vec3>()(vertex.pos);
			size_t h2 = hash<glm::vec3>()(vertex.color);
			size_t h3 = hash<glm::vec2>()(vertex.texture_coords);
			size_t h4 = hash<glm::vec3>()(vertex.normal);

			return (((((((h2 << 1) ^ h1) >> 1) ^ h3) << 1) ^ h4));
		}
	};
}
