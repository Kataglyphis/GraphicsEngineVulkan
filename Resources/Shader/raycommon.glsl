
struct HitPayload {

	vec3 hit_value;

};

struct Vertex {

	vec3 pos;
	vec2 texture_coords;
	vec3 normal;

};

struct ObjectDescription {

	uint64_t texture_id;
	uint64_t vertex_address;
	uint64_t index_address;
	uint64_t placeholder;

};

struct UboViewProjection {

	mat4 projection;
	mat4 view;

};

struct UboDirections {

	vec4 light_dir;
	vec4 view_dir;

};

// Push constant structure for the raster
struct PushConstantRaster
{

	mat4  model;  // matrix of the instance

};

struct PushConstantRay {

	vec4 clear_color;

};
