#version 460

#extension GL_ARB_shading_language_include : require

#include "/host_device_shared.hpp"

layout(location = 2) out vec3 g_albedo;
layout(location = 3) out vec3  g_material_id;

in vec3 tex_coords;
in vec4 world_pos;

uniform samplerCube skybox;

void main()
  {

    g_albedo = texture(skybox, tex_coords).xyz;
    g_material_id = vec3(SKYBOX_MATERIAL_ID);

  }