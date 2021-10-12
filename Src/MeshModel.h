#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <assimp/scene.h>

#include "Mesh.h"

class MeshModel
{
public:

	MeshModel();
	MeshModel(std::vector<Mesh> new_mesh_list);

	size_t get_mesh_count();
	Mesh* get_mesh(size_t index);

	glm::mat4 get_model();
	void set_model(glm::mat4 model);

	void destroy_mesh_model();

	static std::vector<std::string> load_materials(const aiScene* scene);

	~MeshModel();

private:

	std::vector<Mesh> meshes;
	glm::mat4 model;

};

