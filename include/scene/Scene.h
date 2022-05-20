#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <vector>
#include <memory>
#include <cstring>
#include <iostream>
#include <set>
#include <algorithm>
#include <array>
#include <stdlib.h>
#include <stdio.h>

#include "Window.h"
#include "Utilities.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "GUI.h"
#include "GUISceneSharedVars.h"

#include <string>

class Scene
{

public:
	
	Scene();

	void						update_user_input(GUI* gui);

	const GUISceneSharedVars&	getGuiSceneSharedVars() { return guiSceneSharedVars; };

	uint32_t					get_model_count();
	glm::mat4					get_model_matrix(int model_index);
	uint32_t					get_mesh_count(int model_index);
	VkBuffer					get_vertex_buffer(int model_index, int mesh_index);
	VkBuffer					get_index_buffer(int model_index, int mesh_index);
	uint32_t					get_index_count(int model_index, int mesh_index);
	uint32_t					get_number_of_object_descriptions();
	uint32_t					get_number_of_meshes();

	std::vector<std::shared_ptr<Model>> const & get_model_list();
	std::vector<ObjectDescription>				get_object_descriptions();

	void		add_model(std::shared_ptr<Model> model);
	void		add_object_description(ObjectDescription object_description);

	void update_model_matrix(glm::mat4 model_matrix, int model_id);

	void clean_up();

	~Scene();

private:

	std::vector<ObjectDescription>		object_descriptions;
	std::vector<std::shared_ptr<Model>> model_list;

	GUISceneSharedVars					guiSceneSharedVars;

};

