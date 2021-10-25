#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "raycommon.glsl"

layout(location = 0) rayPayloadEXT HitPayload payload;

layout(push_constant) uniform PushConstantRay pc_ray;

void main() {

	payload.hit_value = pc_ray.clear_color.xyz * 0.8f;
    
}