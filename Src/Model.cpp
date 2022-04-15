#include "Model.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Model::Model()
{
}

Model::Model(std::vector<Mesh> new_mesh_list, uint32_t index)
{
    //meshes = new_mesh_list;
    model = glm::mat4(1.0f);
    mesh_model_index = index;
}

void Model::load_model_in_ram(VkPhysicalDevice new_physical_device, VkDevice new_device, VkQueue transfer_queue,
                                VkCommandPool command_pool, std::string model_path, std::vector<int> matToTex) {

    tinyobj::ObjReaderConfig reader_config;
    //reader_config.mtl_search_path = ""; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(model_path, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(EXIT_FAILURE);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    std::unordered_map<Vertex, uint32_t> vertices_map{};
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                glm::vec3 pos = { vx,vy,vz };

                glm::vec3 normals(0.0f);
                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    normals = glm::vec3(nx, ny, nz);
                }

                glm::vec2 tex_coords(0.0f);
                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    tex_coords = glm::vec2(tx, ty);
                }

                Vertex vert;// (pos, tex_coords, normals);
                vert.pos = pos;
                vert.texture_coords = tex_coords;
                vert.normal = normals;
                vert.mat_id.x = matToTex[shapes[s].mesh.material_ids[f]];
                //if(shapes[s].mesh.material_ids[f] == 24) std::cout << shapes[s].mesh.material_ids[f] << endl;
                /* vertices.push_back(vert);
                indices.push_back(indices.size());*/

                if (vertices_map.count(vert) == 0) {

                    vertices_map[vert] = vertices.size();
                    vertices.push_back(vert);

                }

                indices.push_back(vertices_map[vert]);


            }

            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }

    mesh = Mesh(new_device, new_physical_device, transfer_queue,
                               command_pool, &vertices, &indices);
    //// loop over shapes
    //std::unordered_map<Vertex, uint32_t> vertices_map{};

    //for (size_t s = 0; s < shapes.size(); s++) {

    //    std::vector<Vertex> vertices;
    //    std::vector<unsigned int> indices;

    //    for (const auto& index : shapes[s].mesh.indices) {

    //        // access to vertex
    //        tinyobj::index_t idx = index;
    //        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
    //        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
    //        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
    //        glm::vec3 pos = { vx,vy,vz };

    //        // Check if `normal_index` is zero or positive. negative = no normal data
    //        glm::vec3 normals(0.0f);
    //        if (idx.normal_index >= 0) {
    //            tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
    //            tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
    //            tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
    //            normals = glm::vec3(nx, ny, nz);
    //        }

    //        glm::vec2 tex_coords(0.0f);
    //        // Check if `texcoord_index` is zero or positive. negative = no texcoord data
    //        if (idx.texcoord_index >= 0) {

    //            tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
    //            tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
    //            tex_coords = glm::vec2(tx, ty);

    //        }

    //        Vertex vert;// (pos, tex_coords, normals);
    //        vert.pos = pos;
    //        vert.texture_coords = tex_coords;
    //        vert.normal = normals;
    //        /* vertices.push_back(vert);
    //         indices.push_back(indices.size());*/

    //        if (vertices_map.count(vert) == 0) {

    //            vertices_map[vert] = vertices.size();
    //            vertices.push_back(vert);

    //        }

    //        indices.push_back(vertices_map[vert]);

    //    }

    //    /*vertices_per_shape.push_back(vertices);
    //    indices_per_shape.push_back(indices);

    //    shapes_to_material.push_back(shapes[s].mesh.material_ids[0]);*/
    //    meshes.emplace_back( new_device, new_physical_device, transfer_queue,
    //                        command_pool, &vertices, &indices);
    //}

}

ObjectDescription Model::get_object_description()
{
    return mesh.get_object_description();
}

std::vector<std::string> Model::load_textures(std::string modelFile)
{
    tinyobj::ObjReaderConfig reader_config;
    //reader_config.mtl_search_path = ""; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(modelFile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(EXIT_FAILURE);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    texture_list.reserve(materials.size());

    // we now iterate over all materials to get diffuse textures

    for (size_t i = 0; i < materials.size(); i++) {
        const tinyobj::material_t* mp = &materials[i];

        if (mp->diffuse_texname.length() > 0) {

            std::string relative_texture_filename = mp->diffuse_texname;
            std::string texture_filename = get_base_dir(modelFile) + "/textures/" + relative_texture_filename;

            texture_list.push_back(texture_filename);

            /*if (!texture_list[num_tex]->load_texture_without_alpha_channel()) {
                printf("Failed to load texture at: %s\n", texture_filename.c_str());
                delete texture_list[i];
                texture_list[i] = nullptr;
            }*/

        }
        else {

            texture_list.push_back("");

        }
    }

    return texture_list;

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

std::string Model::get_base_dir(const std::string& filepath) {

    if (filepath.find_last_of("/\\") != std::string::npos)
        return filepath.substr(0, filepath.find_last_of("/\\"));
    return "";

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



