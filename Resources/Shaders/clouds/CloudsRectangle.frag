#version 460

#extension GL_ARB_shading_language_include : require

#include "/host_device_shared.hpp"

layout(location = 0) out vec3 g_position;
layout(location = 3) out vec3 g_material_id;

in vec4 cloud_world_pos;

void main() {
	
	g_position		= cloud_world_pos.xyz;
	g_material_id	= vec3(CLOUDS_MATERIAL_ID);

}