#pragma once
#include <glm/glm.hpp>
// illumination model (see http://www.fileformat.info/format/material/)

class ObjMaterial
{
public:

	ObjMaterial();
	ObjMaterial(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 transmittance,
				glm::vec3 emission, float shininess, float ior, float dissolve,
				int illum, int textureID);

	alignas(16) glm::vec3 ambient		= glm::vec3(0.1f, 0.1f, 0.1f);
	glm::vec3 diffuse		= glm::vec3(0.7f, 0.7f, 0.7f);
	glm::vec3 specular		= glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 transmittance = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 emission		= glm::vec3(0.0f, 0.0f, 0.10);
	float         shininess = 0.f;
	float         ior		= 1.0f;  // index of refraction
	float         dissolve	= 1.f;   // 1 == opaque; 0 == fully transparent
							 
	int illum				= 0;
	int textureID			= -1;

private:
};

