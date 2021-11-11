#version 460

#extension GL_EXT_ray_tracing : require

//#include "raycommon.glsl"

layout(location = 1) rayPayloadInEXT bool isShadow;

void main() {
	
	isShadow = false;
  
}