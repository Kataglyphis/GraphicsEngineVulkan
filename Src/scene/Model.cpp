#include "Model.h"

Model::Model()
{
}

Model::Model(std::vector<Mesh> new_mesh_list, uint32_t index)
{
    //meshes = new_mesh_list;
    model = glm::mat4(1.0f);
    mesh_model_index = index;
}

void Model::add_new_mesh(   VulkanDevice* device, 
                            VkQueue transfer_queue, VkCommandPool command_pool, 
                            std::vector<Vertex>& vertices, 
                            std::vector<unsigned int>& indices,
                            std::vector<unsigned int>&	materialIndex, 
                            std::vector<ObjMaterial>& materials)
{

    this->mesh = Mesh(  device, transfer_queue, command_pool, 
                        vertices, indices, materialIndex, materials);

}

void Model::set_model(glm::mat4 model)
{
    this->model = model;
}

void Model::addTexture(Texture newTexture)
{
    modelTextures.push_back(newTexture);
}

uint32_t Model::getPrimitiveCount()
{
    uint32_t number_of_indices = 0;

    /*for (Mesh mesh : meshes) {

        number_of_indices += mesh.get_index_count();

    }*/

    //return number_of_indices / 3;
    return mesh.getIndexCount() / 3;
}

Model::~Model()
{
}



