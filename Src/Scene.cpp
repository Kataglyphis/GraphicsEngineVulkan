#include "Scene.h"

Scene::Scene() {

}

uint32_t Scene::get_model_count()
{
	return static_cast<uint32_t>(model_list.size());
}

void Scene::add_model(Model model)
{
	model_list.push_back(model);

}

void Scene::add_object_description(ObjectDescription object_description)
{

	object_descriptions.push_back(object_description);

}

std::vector<Model> const & Scene::get_model_list()
{
	return model_list;
}

void Scene::update_model_matrix(glm::mat4 model_matrix, int model_id)
{

	model_list[model_id].set_model(model_matrix);

}

void Scene::clean_up()
{

	for (size_t i = 0; i < model_list.size(); i++) {

		model_list[i].destroy_model();

	}

}

glm::mat4 Scene::get_model_matrix(int model_index)
{

	return model_list[model_index].get_model();

}

uint32_t Scene::get_mesh_count(int model_index)
{

	return static_cast<uint32_t>(model_list[model_index].get_mesh_count());

}

VkBuffer Scene::get_vertex_buffer(int model_index, int mesh_index)
{
	return model_list[model_index].get_mesh(mesh_index)->get_vertex_buffer();
}

VkBuffer Scene::get_index_buffer(int model_index, int mesh_index)
{
	return model_list[model_index].get_mesh(mesh_index)->get_index_buffer();
}

uint32_t Scene::get_index_count(int model_index, int mesh_index)
{
	return model_list[model_index].get_mesh(mesh_index)->get_index_count();
}

uint32_t Scene::get_number_of_object_descriptions()
{
	return static_cast<uint32_t>(object_descriptions.size());
}

uint32_t Scene::get_number_of_meshes()
{
	uint32_t number_of_meshes = 0;
	
	for (Model mesh_model : model_list) {
		number_of_meshes += static_cast<uint32_t>(mesh_model.get_mesh_count());
	}

	return number_of_meshes;
}

std::vector<ObjectDescription> Scene::get_object_descriptions()
{
	return object_descriptions;
}

Scene::~Scene() {

}

