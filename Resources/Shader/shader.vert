#version 450								// use GLSL 4.5
// #extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec2 tex_coords;

layout (set = 0, binding = 0) uniform UboViewProjection {
															mat4 projection;
															mat4 view;
} ubo_view_projection;

// NOT IN USE, LEFT FOR REFERENCE 
layout (set = 0, binding = 1) uniform UboModel {
															mat4 model;
} ubo_model;

layout (push_constant) uniform PushModel {

	mat4 model;

} push_model;

layout (location = 0) out vec2 texture_coordinates;

void main () {
	
	// -- WE ARE CALCULATION THE MVP WITH THE GLM LIBRARY WHO IS DESIGNED FOR OPENGL
	// -- THEREFORE TAKE THE DIFFERENT COORDINATE SYSTEMS INTO ACCOUNT
	vec4 opengl_position = ubo_view_projection.projection * ubo_view_projection.view * push_model.model * vec4(positions, 1.0f);
	vec4 vulkan_position = vec4(opengl_position.x, -opengl_position.y, opengl_position.z, opengl_position.w);
	gl_Position = vulkan_position;

	texture_coordinates = tex_coords;

}