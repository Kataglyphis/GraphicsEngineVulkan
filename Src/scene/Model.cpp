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

void Model::add_new_mesh(   VkPhysicalDevice physical_device, VkDevice logical_device, VkQueue transfer_queue,
                            VkCommandPool command_pool, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                            std::vector<unsigned int>&	materialIndex, std::vector<ObjMaterial>& materials)
{

    this->mesh = Mesh(physical_device, logical_device, transfer_queue, command_pool, vertices, indices, materialIndex, materials);

}

ObjectDescription Model::get_object_description()
{
    return mesh.get_object_description();
}

size_t Model::get_mesh_count()
{
    return 1;// meshes.size();
}

Mesh* Model::get_mesh(size_t index)
{

    /*if (index >= meshes.size()) {

        throw std::runtime_error("Attempted to access invalid mesh index!");

    }*/

    return &mesh;//&meshes[index];
}

glm::mat4 Model::get_model()
{
    return model;
}

void Model::set_model(glm::mat4 model)
{
    this->model = model;
}

uint32_t Model::get_custom_instance_index()
{
    return mesh_model_index;
}

uint32_t Model::get_primitive_count()
{
    uint32_t number_of_indices = 0;

    /*for (Mesh mesh : meshes) {

        number_of_indices += mesh.get_index_count();

    }*/

    //return number_of_indices / 3;
    return mesh.get_index_count() / 3;
}

std::vector<std::string> Model::get_texture_list()
{
    return texture_list;
}

void Model::destroy_model()
{
    //for (auto& mesh : meshes) {

        mesh.destroy_buffers();

    //}
}

Model::~Model()
{
}



