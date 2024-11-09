#version 460

#extension GL_ARB_shading_language_include : require

#include "/host_device_shared.hpp"

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec2 in_tex_coord;

out vec2	tex_coords;
out vec3	frag_pos;
out vec3	normal;

//uniform variables
uniform mat4 projection;                
uniform mat4 view;                                                            
uniform mat4 model;
uniform mat4 normal_model;

void main() {
	
	vec4 world_pos = model * vec4(in_position, 1.0);
	frag_pos = world_pos.xyz;
	tex_coords = in_tex_coord;

	gl_Position = projection * view * model * vec4(in_position, 1.0);
	//move matrix calculations to application part 
	normal = mat3(normal_model) * in_normal;

}