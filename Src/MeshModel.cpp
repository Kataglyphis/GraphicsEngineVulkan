#include "MeshModel.h"

MeshModel::MeshModel()
{
}

MeshModel::MeshModel(std::vector<Mesh> new_mesh_list, uint32_t index) 
{
    meshes = new_mesh_list;
    model = glm::mat4(1.0f);
    mesh_model_index = index;
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

uint32_t MeshModel::get_custom_instance_index()
{
    return mesh_model_index;
}

uint32_t MeshModel::get_primitive_count()
{
    uint32_t number_of_indices = 0;

    for (Mesh mesh : meshes) {

        number_of_indices += mesh.get_index_count();

    }

    return number_of_indices / 3;
}

void MeshModel::destroy_mesh_model()
{
    for (auto& mesh : meshes) {

        mesh.destroy_buffers();

    }
}

std::vector<std::string> MeshModel::load_materials(const aiScene* scene)
{
    
    // create 1:1 sized list of texture
    std::vector<std::string> texture_list(scene->mNumMaterials);

    // go through each material and copy its texture file name (if its exists)
    for (size_t i = 0; i < scene->mNumMaterials; i++) {

        // get the material
        aiMaterial* material = scene->mMaterials[i];

        // initialize the texture to empty string (will be replaced if texture exists)
        texture_list[i] = "";

        // check for a diffuse texture (standard detail texture)
        if (material->GetTextureCount(aiTextureType_DIFFUSE)) {

            // get the path of texture file
            aiString path;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {

                // cut off any directory information already present
                // remove really dirty hard coded paths ...
                int idx = std::string(path.data).find("\\");
                std::string file_name = std::string(path.data).substr(idx + 1); 

                texture_list[i] = file_name;

            }

        }

    }

    return texture_list;

}

std::vector<Mesh> MeshModel::load_node(VkPhysicalDevice new_physical_device, VkDevice new_device, VkQueue transfer_queue,
                                                                            VkCommandPool command_pool, aiNode* node, const aiScene* scene, 
                                                                            std::vector<int> mat_to_tex, bool flip_y)
{

    std::vector<Mesh> mesh_list;

    // go through ech mesh at this node and create it, then add it to our mesh list
    for (size_t i = 0; i < node->mNumMeshes; i++) {

        // load mesh here
        mesh_list.push_back(
                            load_mesh(new_physical_device, new_device, transfer_queue,
                                        command_pool, scene->mMeshes[node->mMeshes[i]],
                                        scene, mat_to_tex, flip_y)
                            );

    }

    // go through each node attached to this node and load it, then append their meshes
    // to this node's mesh list
    for (size_t i = 0; i < node->mNumChildren; i++) {

        std::vector<Mesh> new_list = load_node(new_physical_device, new_device, transfer_queue,
                                                                            command_pool, node->mChildren[i], scene,
                                                                            mat_to_tex, flip_y);

        mesh_list.insert(mesh_list.end(), new_list.begin(), new_list.end());

    }

    return mesh_list;

}

Mesh MeshModel::load_mesh(VkPhysicalDevice new_physical_device, VkDevice new_device, VkQueue transfer_queue, 
                                                        VkCommandPool command_pool, aiMesh* mesh, const aiScene* scene, 
                                                        std::vector<int> mat_to_tex, bool flip_y)
{

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // resize vertex list to hold all vertices for mesh
    vertices.resize(mesh->mNumVertices);

    // go through each vertex and copy it across to our vertices 
    for(size_t i = 0; i < mesh->mNumVertices; i++) {

        // set position
        vertices[i].pos = {mesh->mVertices[i].x,
                            mesh->mVertices[i].y,
                            mesh->mVertices[i].z};

        // if (flip_y) vertices[i].pos.y *= -1;

        // set tex coords
        if(mesh->mTextureCoords[0]) {
            
            vertices[i].texture_coords = {mesh->mTextureCoords[0][i].x,
                                            mesh->mTextureCoords[0][i].y};

        } else {

            vertices[i].texture_coords = {0.0f, 0.0f};

        }

        if (mesh->HasNormals()) {

            vertices[i].normal = { mesh->mNormals[i].x,
                                                    mesh->mNormals[i].y,
                                                    mesh->mNormals[i].z };

            if (flip_y) vertices[i].normal.y *= -1;

        } 

    }

    // iterate over indices through faces and copy across
    for(size_t i = 0; i < mesh->mNumFaces; i++) {

        // go through face's indices and add to list
        aiFace face = mesh->mFaces[i];
        for(size_t m = 0; m < face.mNumIndices; m++) {

            indices.push_back(face.mIndices[m]);

        }


    }

    // create new mesh with details and return it
    Mesh new_mesh = Mesh(new_device, new_physical_device, new_device, transfer_queue, 
                            command_pool, &vertices, &indices, 
                            mat_to_tex[mesh->mMaterialIndex]);

    return new_mesh;
}

MeshModel::~MeshModel()
{
}
