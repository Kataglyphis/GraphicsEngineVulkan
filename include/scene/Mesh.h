#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Utilities.h"
#include "Vertex.h"
#include "ObjMaterial.h"
#include "ObjectDescription.h"

// this a simple Mesh without mesh generation
class Mesh {
public:

    Mesh(   VulkanDevice* device, 
            VkQueue transfer_queue, VkCommandPool transfer_command_pool, 
            std::vector<Vertex>& vertices, 
            std::vector<uint32_t>& indices,
            std::vector<unsigned int>& materialIndex, 
            std::vector<ObjMaterial>&	materials);
    
    Mesh();

    ObjectDescription&  get_object_description() { return object_description; };
    glm::mat4           get_model() { return model; };
    uint32_t            get_vertex_count() { return vertex_count; };
    uint32_t            get_index_count() { return index_count; };
    VkBuffer&           get_vertex_buffer() { return vertexBuffer.getBuffer(); };
    VkBuffer&           get_materialID_buffer() { return materialIdsBuffer.getBuffer(); };
    VkBuffer&           get_index_buffer() { return indexBuffer.getBuffer(); };

    void                set_model(glm::mat4 new_model);

     ~Mesh();

private:

    ObjectDescription   object_description;

    VulkanBuffer        vertexBuffer;
    VulkanBuffer        indexBuffer;
    VulkanBuffer        objectDescriptionBuffer;
    VulkanBuffer        materialIdsBuffer;
    VulkanBuffer        materialsBuffer;

    glm::mat4           model;

    uint32_t            vertex_count;
    uint32_t            index_count;


    VulkanDevice*       device;

    void create_vertex_buffer(  VkQueue transfer_queue, VkCommandPool transfer_command_pool, 
                                std::vector<Vertex>* vertices);

    void create_index_buffer(   VkQueue transfer_queue, VkCommandPool transfer_command_pool, 
                                std::vector<uint32_t>* indices);

    void create_material_id_buffer( VkQueue transfer_queue, VkCommandPool transfer_command_pool, 
                                    std::vector<unsigned int>* materialIndex);

    void create_material_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, 
                                std::vector<ObjMaterial>* materials);

};



