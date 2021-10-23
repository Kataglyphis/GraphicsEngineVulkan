#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <assimp/scene.h>

#include "Mesh.h"

class MeshModel
{
public:

	MeshModel();
	MeshModel(std::vector<Mesh> new_mesh_list, uint32_t index);

	size_t get_mesh_count();
	Mesh* get_mesh(size_t index);

	glm::mat4 get_model();
	void set_model(glm::mat4 model);

	uint32_t get_custom_instance_index();

	void destroy_mesh_model();

	static std::vector<std::string> load_materials(const aiScene* scene);

	static std::vector<Mesh>  load_node(VkPhysicalDevice new_physical_device, VkDevice new_device, 
																VkQueue transfer_queue, VkCommandPool command_pool,
																aiNode* node, const aiScene* scene, 
																std::vector<int> mat_to_tex, bool flip_y);

	static Mesh load_mesh(VkPhysicalDevice new_physical_device, VkDevice new_device,
											VkQueue transfer_queue, VkCommandPool command_pool,
											aiMesh* mesh, const aiScene* scene,
											std::vector<int> mat_to_tex, bool flip_y);

	~MeshModel();

private:

	uint32_t mesh_model_index;
	std::vector<Mesh> meshes;
	glm::mat4 model;

};

