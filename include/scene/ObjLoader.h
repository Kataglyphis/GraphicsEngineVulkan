#pragma once
#include <vulkan/vulkan.h>
#include <memory>

#include "Model.h"
#include "Vertex.h"
#include "ObjMaterial.h"

class ObjLoader {
  public:
  ObjLoader(VulkanDevice* device, VkQueue transfer_queue, VkCommandPool command_pool);

  std::shared_ptr<Model> loadModel(const std::string& modelFile);


  private:
  VulkanDevice* device;
  VkQueue transfer_queue;
  VkCommandPool command_pool;

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<ObjMaterial> materials;
  std::vector<unsigned int> materialIndex;
  std::vector<std::string> textures;

  std::vector<std::string> loadTexturesAndMaterials(const std::string& modelFile);
  void loadVertices(const std::string& fileName);
};
