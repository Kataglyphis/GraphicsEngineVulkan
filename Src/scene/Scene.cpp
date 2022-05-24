#include "Scene.h"

Scene::Scene() {

}

void Scene::update_user_input(GUI* gui)
{
	guiSceneSharedVars = gui->getGuiSceneSharedVars();
}

void Scene::add_model(std::shared_ptr<Model> model)
{
	model_list.push_back(model);
	object_descriptions.push_back(model->get_object_description());
}

void Scene::add_object_description(ObjectDescription object_description)
{

	object_descriptions.push_back(object_description);

}

void Scene::update_model_matrix(glm::mat4 model_matrix, int model_id)
{

	if (model_id >= static_cast<int32_t>(getModelCount()) || model_id < 0) {

		throw std::runtime_error("Wrong model id value!");

	}

	model_list[model_id]->set_model(model_matrix);

}

uint32_t Scene::getNumberMeshes()
{
	uint32_t number_of_meshes = 0;
	
	for (std::shared_ptr<Model> mesh_model : model_list) {
		number_of_meshes += static_cast<uint32_t>(mesh_model->get_mesh_count());
	}

	return number_of_meshes;
}

Scene::~Scene() {

}

