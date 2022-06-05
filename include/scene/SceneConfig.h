#pragma once
#include <string>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace sceneConfig {

std::string getModelFile();
glm::mat4 getModelMatrix();

}
