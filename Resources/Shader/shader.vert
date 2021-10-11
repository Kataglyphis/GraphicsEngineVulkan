#version 450								// use GLSL 4.5
// #extension GL_KHR_vulkan_glsl : enable

layout(location=0) in vec3 positions; 

layout(binding = 0) uniform UboViewProjection {
															mat4 projection;
															mat4 view;
} ubo_view_projection;

// NOT IN USE, LEFT FOR REFERENCE 
layout(binding = 1) uniform UboModel {
															mat4 model;
} ubo_model;

layout(push_constant) uniform PushModel {

	mat4 model;

} push_model;

void main () {

	gl_Position = ubo_view_projection.projection * ubo_view_projection.view * push_model.model * vec4(positions, 1.0f);

}