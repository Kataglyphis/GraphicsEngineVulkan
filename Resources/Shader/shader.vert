// #extension GL_KHR_vulkan_glsl : enable

#version 460																										// use GLSL 4.6
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "raycommon.glsl"
#include "SetsAndBindings.glsl"

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec3 normal;
layout (location = 3) in uint matID;

layout (set = 0, binding = UBO_VIEW_PROJECTION_BINDING) uniform _UboViewProjection {
	UboViewProjection ubo_view_projection;
};

layout (set = 0, binding = UBO_DIRECTIONS_BINDING) uniform _UboDirections {
	UboDirections ubo_directions;
};

layout (push_constant) uniform _PushConstantRaster {
	PushConstantRaster pc_raster;
};

layout (location = 0) out vec2 texture_coordinates;
layout (location = 1) out vec3 shading_normal;
layout (location = 2) flat out uint fragMaterialID;

out gl_PerVertex
{
  vec4 gl_Position;
};

void main () {
	
	// -- WE ARE CALCULATION THE MVP WITH THE GLM LIBRARY WHO IS DESIGNED FOR OPENGL
	// -- THEREFORE TAKE THE DIFFERENT COORDINATE SYSTEMS INTO ACCOUNT
	vec4 opengl_position = ubo_view_projection.projection * ubo_view_projection.view * pc_raster.model * vec4(positions, 1.0f);
	vec4 vulkan_position = vec4(opengl_position.x, -opengl_position.y, opengl_position.z, opengl_position.w);

	shading_normal = vec3(transpose(inverse(pc_raster.model)) * vec4(normal, 1.0f));
	texture_coordinates = tex_coords;

	fragMaterialID = matID;

	gl_Position = vulkan_position;
}