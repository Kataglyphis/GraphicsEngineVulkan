#pragma once
#include <glm/glm.hpp>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class Vertex
{
public:
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 mat_id;
	glm::vec2 texture_coords;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && normal == other.normal && texture_coords == other.texture_coords;
	}

};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texture_coords) << 1);
		}
	};
}
