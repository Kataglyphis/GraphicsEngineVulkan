#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>

#include "Mesh.h"

#include <memory>

class Model
{
public:

    Model();
    Model(std::vector<Mesh> new_mesh_list, uint32_t index);

    void add_new_mesh(  VulkanDevice* device, 
                        VkQueue transfer_queue, VkCommandPool command_pool, 
                        std::vector<Vertex>& vertices, 
                        std::vector<unsigned int>& indices,
                        std::vector<unsigned int>&	materialIndex, 
                        std::vector<ObjMaterial>&	materials);

    size_t                      get_mesh_count();
    Mesh*                       get_mesh(size_t index);
    glm::mat4                   get_model();
    uint32_t                    get_custom_instance_index();
    uint32_t                    get_primitive_count();
    std::vector<std::string>    get_texture_list();
    ObjectDescription           get_object_description();

    void                        set_model(glm::mat4 model);
   
    ~Model();

private:

    uint32_t                    mesh_model_index;
    Mesh                        mesh;
    glm::mat4                   model;

    std::vector<std::string>    texture_list;
};
