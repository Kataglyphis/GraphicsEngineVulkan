#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "raycommon.glsl"
#include "SetsAndBindings.glsl"
#include "GlobalValues.glsl"

//hitAttributeEXT vec2 attribs;
//
//layout(location = 0) rayPayloadInEXT HitPayload payload;
//layout(location = 1) rayPayloadEXT bool isShadowed;

//layout (set = UBO_DIRECTIONS_SET, binding = UBO_DIRECTIONS_BINDING) uniform _UboDirections {
//    UboDirections ubo_directions;
//};
//
//layout(set = TLAS_SET, binding = TLAS_BINDING) uniform accelerationStructureEXT TLAS;
//layout(set = OBJECT_DESCRIPTION_SET, binding = OBJECT_DESCRIPTION_BINDING, scalar) buffer ObjectDescription_ {
//    ObjectDescription i[];
//} object_description;
//layout(set = TEXTURES_SET, binding = TEXTURES_BINDING) uniform sampler2D texture_sampler[];
//
//
//layout(buffer_reference, scalar) buffer Vertices {
//    Vertex v[]; 
//}; // Positions of an object
//
//layout(buffer_reference, scalar) buffer Indices {
//    ivec3 i[]; 
//}; // Triangle indices

layout(push_constant) uniform _PushConstantRay {
    PushConstantRay pc_ray;
};

void main() {
    
//    ObjectDescription obj_res = object_description.i[gl_InstanceCustomIndexEXT];
//    Indices indices = Indices(obj_res.index_address);
//    Vertices vertices = Vertices(obj_res.vertex_address);
//
//    // indices of triangle 
//    ivec3 ind = indices.i[gl_PrimitiveID];
//
//    // vertex of triangle 
//    Vertex v0 = vertices.v[ind.x];
//    Vertex v1 = vertices.v[ind.y];
//    Vertex v2 = vertices.v[ind.z];
//
//    const vec3 barycentrics = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
//
//    // compte coordinate of hit position 
//    const vec3 hit_pos = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
//    const vec3 world_hit_pos = vec3(gl_ObjectToWorldEXT * vec4(hit_pos, 1.0f));
//
//    //compute normal at hit position 
//    const vec3 normal_hit = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
//    const vec3 world_normal_hit = normalize(vec3(normal_hit * gl_WorldToObjectEXT));
//
//    vec3 L = vec3(ubo_directions.light_dir);
//	vec3 N = normalize(world_normal_hit);
//	vec3 V = normalize(ubo_directions.view_dir.xyz);
//	vec3 R = reflect(L, N);
//
//    vec2 texture_coordinates = v0.texture_coords * barycentrics.x + v1.texture_coords * barycentrics.y + v2.texture_coords * barycentrics.z;
//
//    uint texture_id = uint(object_description.i[gl_InstanceCustomIndexEXT].texture_id);
//	vec3 ambient = texture(texture_sampler[nonuniformEXT(texture_id)], texture_coordinates).xyz;
//	vec3 diffuse = max(dot(N,L),0.0f) * texture(texture_sampler[nonuniformEXT(texture_id)], texture_coordinates).xyz;
//    vec3 specular = vec3(0.f);
//
//    if(dot(world_normal_hit, L) > 0) {
//    
//        float t_min = 0.001;
//        float t_max = 10000;
//        vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
//        vec3 ray_dir = L;
//        uint flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
//        isShadowed = true;
//        traceRayEXT(TLAS,  // acceleration structure
//                                flags,       // rayFlags
//                                0xFF,        // cullMask
//                                0,           // sbtRecordOffset
//                                0,           // sbtRecordStride
//                                1,           // missIndex
//                                origin,      // ray origin
//                                t_min,        // ray min range
//                                ray_dir,      // ray direction
//                                t_max,        // ray max range
//                                1            // payload (location = 1)
//        );
//	    
//        if(!isShadowed) {
//        
//            specular = pow(max(dot(R,V), 0.0f), 8.0) * vec3(1.f);
//
//        } 
//
//    }
//
//    payload.hit_value = ambient * 0.3f + diffuse + specular * 0.00001f; 

}