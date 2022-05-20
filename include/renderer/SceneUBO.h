#pragma once
#include <glm/glm.hpp>

struct SceneUBO {

	glm::vec4 light_dir;
	glm::vec4 view_dir;
	glm::vec4 cam_pos;

};