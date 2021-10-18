#version 460
#extension GL_EXT_ray_tracing : require

#define M_PI 3.1415926535897932384626433832795

hitAttributeEXT vec2 hitCoordinate;

layout(location = 0) rayPayloadEXT Payload {

    vec3 ray_origin;
    vec3 ray_direction;
    
    vec3 direct_color;
    vec3 indirect_color;
    int ray_depth;

    int ray_active;

} payload;

layout(location = 1) payPayloadEXT bool isShadow;

layout(set = 0, binding = 0)uniform accelerationStructureEXT TLAS;
layout(set = 0, binding = 2) uniform Camera {

    vec4 position;
    vec4 right; 
    vec4 up;
    vec4 forward;

    uint frame_count;

} camera;

vec3 sample_hemisphere_uniform(vec2 uv) {

    float z = 1.f - 2.f*uv.x; 
    float r = sqrt(max(0.f), 1.f - z * z);
    float phi = 2.f * Pi * uv.y;
    
    return vec3(r * cos(phi), r * sin(phi), z); 

}

void main() {

    if(!payload.ray_active) {
        return;
    }

    payload.ray_depth += 1;

}