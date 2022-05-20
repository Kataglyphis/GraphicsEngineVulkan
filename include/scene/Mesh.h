#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Utilities.h"
#include "Vertex.h"
#include "ObjMaterial.h"
#include "ObjectDescription.h"

using namespace std;

// this a simple Mesh without mesh generation
class Mesh {
public:

    Mesh(   VkPhysicalDevice physical_device, VkDevice logical_device, VkQueue transfer_queue,
            VkCommandPool transfer_command_pool, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
            std::vector<unsigned int>& materialIndex, std::vector<ObjMaterial>&	materials);
    
    Mesh();

    void set_model(glm::mat4 new_model);
    glm::mat4 get_model();
    ObjectDescription get_object_description();

    int get_vertex_count();
    int get_index_count();
    VkBuffer get_vertex_buffer();
    VkBuffer get_materialID_buffer();
    VkBuffer get_index_buffer();

    void destroy_buffers();

     ~Mesh();

private:
    ObjectDescription object_description;

    glm::mat4 model;

    int vertex_count;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    int index_count;
    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;

    VkBuffer object_description_buffer;
    VkDeviceMemory object_description_buffer_memory;

    VkBuffer material_ids_buffer;
    VkDeviceMemory material_ids_buffer_memory;

    VkBuffer materials_buffer;
    VkDeviceMemory materials_buffer_memory;

    VkPhysicalDevice physical_device;
    VkDevice device;

    void create_vertex_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<Vertex>* vertices);
    void create_index_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<uint32_t>* indices);
    void create_material_id_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<unsigned int>* materialIndex);
    void create_material_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<ObjMaterial>* materials);

};



