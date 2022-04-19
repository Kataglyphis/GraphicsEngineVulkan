#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "../common/raycommon.glsl"
#include "../common/SetsAndBindings.glsl"
#include "../common/GlobalValues.glsl"
#include "../common/ShadingLibrary.glsl"

hitAttributeEXT vec2 attribs;

layout(location = 0) rayPayloadInEXT HitPayload payload;
layout(location = 1) rayPayloadEXT bool isShadowed;

layout (set = UBO_DIRECTIONS_SET, binding = UBO_DIRECTIONS_BINDING) uniform _UboDirections {
    UboDirections ubo_directions;
};

layout(set = TLAS_SET, binding = TLAS_BINDING) uniform accelerationStructureEXT TLAS;
layout(set = OBJECT_DESCRIPTION_SET, binding = OBJECT_DESCRIPTION_BINDING, scalar) buffer ObjectDescription_ {
    ObjectDescription i[];
} object_description;

layout(set = 1, binding = SAMPLER_BINDING_RT) uniform sampler texture_sampler;
layout(set = 1, binding = TEXTURES_BINDING) uniform texture2D tex[TEXTURE_COUNT];

layout(buffer_reference, scalar) buffer Vertices {
    Vertex v[]; 
}; // Positions of an object

layout(buffer_reference, scalar) buffer Indices {
    ivec3 i[]; 
}; // Triangle indices

layout(push_constant) uniform _PushConstantRay {
    PushConstantRay pc_ray;
};

void main() {
    
    const vec3 barycentrics = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

    ObjectDescription obj_res = object_description.i[gl_InstanceCustomIndexEXT];
    Indices indices = Indices(obj_res.index_address);
    Vertices vertices = Vertices(obj_res.vertex_address);

    // indices of triangle 
    ivec3 ind = indices.i[gl_PrimitiveID];

    // vertex of triangle 
    Vertex v0 = vertices.v[ind.x];
    Vertex v1 = vertices.v[ind.y];
    Vertex v2 = vertices.v[ind.z];

    v0.pos.y *= -1;
    v1.pos.y *= -1;
    v2.pos.y *= -1;


    // compte coordinate of hit position 
    const vec3 hit_pos = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
    const vec3 world_hit_pos = vec3(gl_ObjectToWorldEXT * vec4(hit_pos, 1.0f));

    //compute normal at hit position 
    const vec3 normal_hit = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
    const vec3 world_normal_hit = normalize(vec3(gl_ObjectToWorldEXT * vec4(normal_hit,1.0f)));

    vec2 texture_coordinates =  v0.texture_coords * barycentrics.x +
                                v1.texture_coords * barycentrics.y +
                                v2.texture_coords * barycentrics.z;

    //uint texture_id = v0.mat_id;//uint(object_description.i[gl_InstanceCustomIndexEXT].texture_id);
    vec3 ambient = texture(sampler2D(tex[nonuniformEXT(uint(v1.mat_id[0]))], texture_sampler), texture_coordinates).xyz;

    vec3 L = normalize(vec3(-ubo_directions.light_dir)); 
    // no need to normalize
	vec3 N = normalize(normal_hit);
	vec3 V = normalize(hit_pos - ubo_directions.cam_pos.xyz);

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
    
    payload.hit_value = vec3(0.f);//ambient / PI;//

    float roughness = 0.1;
    vec3 light_color = vec3(1.f);
    float light_intensity = 1.f;
    float cosTheta = dot(L,N);
    int mode = 1;
    if(cosTheta > 0) {
        // mode :
        // [0] --> EPIC GAMES (https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf)
        // [1] --> PBR BOOK (https://pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models)
	    payload.hit_value += light_color * light_intensity * evaluateCookTorrenceBRDF(ambient, N, L, V, roughness, mode) * cosTheta;
    }


}