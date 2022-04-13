 #version 460																									

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_scalar_block_layout : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "raycommon.glsl"
#include "SetsAndBindings.glsl"
#include "ShadingLib.glsl"
#include "GlobalValues.glsl"

//layout (push_constant) uniform PushConstantRaster {
//
//	mat4  model;
//
//} pc_raster;

layout (location = 0) in vec2 texture_coordinates;
layout (location = 1) in vec3 shading_normal;
layout (location = 2) flat in uint fragMaterialID;

layout (set = 0, binding = UBO_DIRECTIONS_BINDING) uniform _UboDirections {
	UboDirections ubo_directions;
};

layout (location = 0) out vec4 color;																	//final output color (must have location)

//layout(set = 1, binding = 0) uniform sampler2D texture_sampler;
layout(set = 1, binding = 0) uniform sampler texture_sampler;
layout(set = 1, binding = 1) uniform texture2D tex[TEXTURE_COUNT];

void main() {
	
	vec3 L = normalize(vec3(ubo_directions.light_dir)); // 
	vec3 N = normalize(shading_normal);
	vec3 V = normalize(ubo_directions.view_dir.xyz);
	vec3 R = reflect(L, N);
	vec3 H = normalize(V+L);

	vec3 ambient = texture(sampler2D(tex[fragMaterialID], texture_sampler), texture_coordinates).xyz;
	vec3 diffuse = max(dot(N,L),0.0f) * texture(sampler2D(tex[fragMaterialID], texture_sampler), texture_coordinates).xyz;
	vec3 specular = pow(max(dot(R,V), 0.0f), 8.0) * vec3(1.f);

	float roughness = 5;
	vec3 light_color = vec3(1.f);
	float light_ambient_intensity = 1.f;

	color = vec4(CookTorrenceBRDF(ambient, N, H, L, V, roughness, 
				light_color, light_ambient_intensity), 1.0f);//vec4(ambient * 0.3f + diffuse + specular * 0.00001f,1.0f);
	//color = vec4(1.0f);

}
