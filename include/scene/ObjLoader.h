#pragma once
#include <vulkan/vulkan.h>
#include <memory>

#include "Model.h"
#include "Material.h"
#include "VulkanRenderer.hpp"

class ObjLoader
{
public:

	ObjLoader(	VkPhysicalDevice physical_device, VkDevice logical_device, VkQueue transfer_queue,
				VkCommandPool command_pool);

	std::shared_ptr<Model> load_model(std::string modelFile, std::vector<int> matToTex);
	std::vector<std::string> load_textures(std::string modelFile);


private:

	VkPhysicalDevice physical_device;
	VkDevice logical_device; 
	VkQueue transfer_queue;
	VkCommandPool command_pool;

	std::vector<Vertex>			vertices;
	std::vector<unsigned int>	indices;
	std::vector<Material>		materials;
	std::vector<unsigned int>	materialIndex;
	std::vector<std::string>	textures;

};

