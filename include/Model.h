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

    size_t get_mesh_count();
    Mesh* get_mesh(size_t index);
    glm::mat4 get_model();
    uint32_t get_custom_instance_index();
    uint32_t get_primitive_count();
    std::vector<std::string> get_texture_list();

    void load_model_in_ram(VkPhysicalDevice new_physical_device, VkDevice new_device, VkQueue transfer_queue,
                            VkCommandPool command_pool, std::string model_path);

    void set_model(glm::mat4 model);

    std::string get_base_dir(const std::string& filepath);

    void destroy_model();
   
    ~Model();

private:

    uint32_t mesh_model_index;
    std::vector<Mesh> meshes;
    glm::mat4 model;

    std::vector<std::string> texture_list;
};
