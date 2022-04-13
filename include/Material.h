#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Material
{
public:

	Material();
	Material(GLfloat metallic, GLfloat roughness, GLfloat IOR, GLfloat absorption_coef);

	void use_material(GLuint uniform_metallic_location, GLuint uniform_roughness_location, 
		GLuint uniform_IOR_location, GLfloat uniform_absorption_coef_location);

	~Material();

private:

	GLfloat metallic;
	GLfloat roughness;
	GLfloat IOR;
	GLfloat absorption_coef;

};

