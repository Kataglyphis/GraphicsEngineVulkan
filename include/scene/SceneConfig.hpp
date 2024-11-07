#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

namespace sceneConfig {

std::string getModelFile();
glm::mat4 getModelMatrix();

}// namespace sceneConfig
