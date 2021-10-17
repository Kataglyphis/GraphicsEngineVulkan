 #version 450								// use GLSL 4.5

layout (location = 0) in vec2 texture_coordinates;
layout (location = 1) in vec3 shading_normal;

layout (location = 0) out vec4 color;		//final output color (must have location)

layout(set = 1, binding = 0) uniform sampler2D texture_sampler;

void main() {
	
	vec3 direct_light = vec3(1.f, 1.f, 1.f);

	float cos_theta = dot(normalize(shading_normal), normalize(direct_light));
	// color = texture(texture_sampler, texture_coordinates) * cos_theta;
	// color = vec4(shading_normal.x, shading_normal.y,shading_normal.z,1.0f);
	// color = vec4(cos_theta, cos_theta, cos_theta, 1.0f);
	color = vec4(0.5f * texture(texture_sampler, texture_coordinates).xyz + vec3(1.f) * cos_theta* 0.3f,1.0f) ;
	//color = vec4(1.0f,0.0f,1.0f,1.0f);

}