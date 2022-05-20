
struct HitPayload {

	vec3 hit_value;

};

struct Vertex {

	vec3 pos;
	vec3 normal;
	vec3 color;
	vec2 texture_coords;

};

struct ObjectDescription {

	uint64_t vertex_address;
	uint64_t index_address;
	uint64_t material_index_address;
	uint64_t material_address;

};

struct GlobalUBO {

	mat4 projection;
	mat4 view;

};

struct SceneUBO {

	vec4 light_dir;
	vec4 view_dir;
	vec4 cam_pos;

};

// Push constant structure for the raster
struct PushConstantRaster
{

	mat4  model;  // matrix of the instance

};

struct PushConstantRay {

	vec4 clear_color;

};

struct ObjMaterial  
{
	vec3	ambient;
	vec3	diffuse;
	vec3	specular;
	vec3	transmittance;
	vec3	emission;
	float	shininess;
	float	ior;       // index of refraction
	float	dissolve;  // 1 == opaque; 0 == fully transparent
	int		illum;     // illumination model (see http://www.fileformat.info/format/material/)
	int		textureId;
};
