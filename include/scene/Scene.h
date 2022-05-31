#pragma once
#define GLFW_INCLUDE_NONE
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
#include "ObjLoader.h"

#include <string>

class Scene
{

public:
	
	Scene();

	void						update_user_input(GUI* gui);
	void						update_model_matrix(glm::mat4 model_matrix, int model_id);

	const GUISceneSharedVars&	getGuiSceneSharedVars() { return guiSceneSharedVars; };

	std::vector<Texture>&	getTextures(int model_index) { return model_list[model_index]->getTextures(); };
	uint32_t				getTextureCount(int model_index) { return model_list[model_index]->getTextureCount(); };
	uint32_t		getModelCount() { return static_cast<uint32_t>(model_list.size()); };
	glm::mat4		getModelMatrix(int model_index) { return model_list[model_index]->getModel(); };
	uint32_t		getMeshCount(int model_index) { return static_cast<uint32_t>(model_list[model_index]->getMeshCount()); };
	VkBuffer		getVertexBuffer(int model_index, int mesh_index) { return model_list[model_index]->getMesh(mesh_index)->getVertexBuffer(); };
	VkBuffer		getIndexBuffer(int model_index, int mesh_index) { return model_list[model_index]->getMesh(mesh_index)->getIndexBuffer(); };
	uint32_t		getIndexCount(int model_index, int mesh_index) { return model_list[model_index]->getMesh(mesh_index)->getIndexCount(); };
	uint32_t		getNumberObjectDescriptions() { return static_cast<uint32_t>(object_descriptions.size()); };
	uint32_t		getNumberMeshes();
	std::vector<ObjectDescription>				getObjectDescriptions() { return object_descriptions; };
	std::vector<std::shared_ptr<Model>> const&	get_model_list() { return model_list; };

	void						loadModel(	VulkanDevice* device,
											VkCommandPool commandPool, 
											std::string modelFileName);

	void						add_model(std::shared_ptr<Model> model);
	void						add_object_description(ObjectDescription object_description);

	void						cleanUp();
	~Scene();

private:

	std::vector<ObjectDescription>		object_descriptions;
	std::vector<std::shared_ptr<Model>> model_list;

	GUISceneSharedVars					guiSceneSharedVars;

};

