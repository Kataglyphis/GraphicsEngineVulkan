#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>

#include "Mesh.h"
#include "Texture.h"

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

    uint32_t                    getTextureCount() { return static_cast<uint32_t>(modelTextures.size()); };
    std::vector<Texture>&       getTextures() { return modelTextures; }
    std::vector<std::string>    getTextureList() { return texture_list; };
    uint32_t                    getMeshCount() { return 1; };
    Mesh*                       getMesh(size_t index) { return &mesh; };
    glm::mat4                   getModel() { return model; };
    uint32_t                    getCustomInstanceIndex() { return mesh_model_index; };
    uint32_t                    getPrimitiveCount();
    ObjectDescription           getObjectDescription() { return mesh.getObjectDescription(); };

    void                        set_model(glm::mat4 model);
    void                        addTexture(Texture newTexture);

    ~Model();

private:

    uint32_t                    mesh_model_index;
    Mesh                        mesh;
    glm::mat4                   model;

    std::vector<std::string>    texture_list;
    std::vector<Texture>        modelTextures;
};
