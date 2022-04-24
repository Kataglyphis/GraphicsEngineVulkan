#include "Vertex.h"

Vertex::Vertex()
{
	this->pos				= glm::vec3(-1.f);
	this->normal			= glm::vec3(-1.f);
	this->color				= glm::vec3(-1.f);
	this->mat_id			= glm::vec3(-1.f);
	this->texture_coords	= glm::vec3(-1.f);
}

Vertex::Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 color, glm::vec3 mat_id, glm::vec2 texture_coords)
{
	this->pos				= pos;
	this->normal			= normal;
	this->color				= color;
	this->mat_id			= mat_id;
	this->texture_coords	= texture_coords;
}
