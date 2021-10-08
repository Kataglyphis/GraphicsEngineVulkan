#version 450								// use GLSL 4.5
// #extension GL_KHR_vulkan_glsl : enable

layout(location=0) in vec3 positions; 

layout(binding = 0) uniform MVP {
															mat4 projection;
															mat4 view;
															mat4 model;
} mvp;

void main () {

	gl_Position = mvp.projection * mvp.view * mvp.model * vec4(positions, 1.0f);

}