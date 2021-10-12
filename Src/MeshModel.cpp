#include "MeshModel.h"

MeshModel::MeshModel()
{
}

MeshModel::MeshModel(std::vector<Mesh> new_mesh_list)
{
    meshes = new_mesh_list;
    model = glm::mat4(1.0f);
}

size_t MeshModel::get_mesh_count()
{
    return meshes.size();
}

Mesh* MeshModel::get_mesh(size_t index)
{

    if (index >= meshes.size()) {

        throw std::runtime_error("Attempted to access invalid mesh index!");

    }

    return &meshes[index];
}

glm::mat4 MeshModel::get_model()
{
    return model;
}

void MeshModel::set_model(glm::mat4 model)
{
    this->model = model;
}

void MeshModel::destroy_mesh_model()
{
    for (auto& mesh : meshes) {

        mesh.destroy_buffers();

    }
}

std::vector<std::string> MeshModel::load_materials(const aiScene* scene)
{
    return std::vector<std::string>();
}

MeshModel::~MeshModel()
{
}
