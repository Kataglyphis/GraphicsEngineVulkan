 #version 460																									

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_scalar_block_layout : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "../common/raycommon.glsl"
#include "../common/SetsAndBindings.glsl"
#include "../common/ShadingLibrary.glsl"
#include "../common/GlobalValues.glsl"

//layout (push_constant) uniform PushConstantRaster {
//
//	mat4  model;
//
//} pc_raster;

layout (location = 0) in vec2 texture_coordinates;
layout (location = 1) in vec3 shading_normal;
layout (location = 2) flat in uint fragMaterialID;
layout (location = 3) in vec3 worldPosition;

layout (set = 0, binding = UBO_DIRECTIONS_BINDING) uniform _UboDirections {
	UboDirections ubo_directions;
};

layout (location = 0) out vec4 out_color;


layout(set = 1, binding = 0) uniform sampler texture_sampler;
layout(set = 1, binding = 1) uniform texture2D tex[TEXTURE_COUNT];

void main() {
	
	vec3 L = normalize(vec3(-ubo_directions.light_dir));
	vec3 N = normalize(shading_normal);
	vec3 V = normalize(worldPosition - ubo_directions.cam_pos.xyz);

	vec3 ambient = texture(sampler2D(tex[fragMaterialID], texture_sampler), texture_coordinates).xyz;

	float roughness = 0.1;
	vec3 light_color = vec3(1.f);
	float light_intensity = 1.f;
	float cosTheta = dot(L,N);
	int mode = 0;
	vec3 color = vec3(0.f);//ambient / PI; //

	if(cosTheta>0) {
		// mode :
		// [0] --> EPIC GAMES (https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf)
		// [1] --> PBR BOOK (https://pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models)
		color += light_color * light_intensity * evaluateCookTorrenceBRDF(ambient, N, L, V, roughness, mode) * cosTheta;
	}

	out_color = vec4(color,1.0);

}
