#include "Vertex.h"

Vertex::Vertex()
{
}

Vertex::Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 color, glm::vec3 mat_id, glm::vec2 texture_coords)
{
	this->pos				= pos;
	this->normal			= normal;
	this->color				= color;
	this->mat_id			= mat_id;
	this->texture_coords	= texture_coords;
}
