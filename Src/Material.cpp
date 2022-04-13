#include "Material.h"

Material::Material() {

	metallic = 0.0f; 
	roughness = 0.0f;
	IOR = 1.f;
	absorption_coef = 0.0f;
}

Material::Material(GLfloat metallic, GLfloat roughness, GLfloat IOR, GLfloat absorption_coef) {

	this->metallic = metallic;
	this->roughness = roughness;
	this->IOR = IOR;
	this->absorption_coef = absorption_coef;

}

void Material::use_material(GLuint uniform_metallic_location, GLuint uniform_roughness_location,
											GLuint uniform_IOR_location, GLfloat uniform_absorption_coef) {


}

Material::~Material() {



}