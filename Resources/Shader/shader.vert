// #extension GL_KHR_vulkan_glsl : enable

#version 450																										// use GLSL 4.6
//#extension gl_arb_separate_shader_objects : enable
//#extension gl_ext_scalar_block_layout : enable
//#extension gl_google_include_directive : enable
//
//#extension gl_ext_shader_explicit_arithmetic_types_int64 : require

//#include "raycommon.glsl"
//#include "SetsAndBindings.glsl"

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec3 normal;

layout (set = 0, binding = 0) uniform UboViewProjection {
	
	mat4 projection;
	mat4 view;

} ubo_view_projection;

layout (set = 0, binding = 1) uniform UboDirections {

	vec3 light_dir;
	vec3 view_dir; 

} ubo_directions;

layout (push_constant) uniform PushConstantRaster {

	mat4 model;

} pc_raster;

layout (location = 0) out vec2 texture_coordinates;
layout (location = 1) out vec3 shading_normal;
layout (location = 2) out vec3 light_dir;
layout (location = 3) out vec3 view_dir;

void main () {
	
	// -- WE ARE CALCULATION THE MVP WITH THE GLM LIBRARY WHO IS DESIGNED FOR OPENGL
	// -- THEREFORE TAKE THE DIFFERENT COORDINATE SYSTEMS INTO ACCOUNT
	vec4 opengl_position = ubo_view_projection.projection * ubo_view_projection.view * pc_raster.model * vec4(positions, 1.0f);
	vec4 vulkan_position = vec4(opengl_position.x, -opengl_position.y, opengl_position.z, opengl_position.w);

	shading_normal = vec3(transpose(inverse(pc_raster.model)) * vec4(normal, 1.0f));
	texture_coordinates = tex_coords;
	light_dir = ubo_directions.light_dir;
	view_dir = ubo_directions.view_dir;

	gl_Position = vulkan_position;
}