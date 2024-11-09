#version 460

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec2 in_tex_coord;

out vec3 tex_coords;

uniform mat4 projection;                
uniform mat4 view;     


void main() {

	tex_coords = in_position;
	vec4 pos = projection * view * vec4(in_position, 1.0);
	gl_Position = pos.xyww;

}