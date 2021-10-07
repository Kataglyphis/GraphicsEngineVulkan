#version 450								// use GLSL 4.5
// #extension GL_KHR_vulkan_glsl : enable

layout(location=0) in vec3 positions; 

void main () {

	gl_Position = vec4(positions, 1.0f);

}