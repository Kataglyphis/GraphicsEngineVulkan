 #version 450								// use GLSL 4.5

layout (location = 0) in vec2 texture_coordinates;
layout (location = 1) in vec3 shading_normal;
layout (location = 2) in vec3 light_dir;
layout (location = 3) in vec3 view_dir;

layout (location = 0) out vec4 color;		//final output color (must have location)

layout(set = 1, binding = 0) uniform sampler2D texture_sampler;

void main() {
	
	vec3 L = vec3(light_dir);
	vec3 N = normalize(shading_normal);
	vec3 V = normalize(view_dir);
	vec3 R = reflect(L, N);

	vec3 ambient = texture(texture_sampler, texture_coordinates).xyz;
	vec3 diffuse = max(dot(N,L),0.0f) * texture(texture_sampler, texture_coordinates).xyz;
	vec3 specular = pow(max(dot(R,V), 0.0f), 8.0) * vec3(1.f);

	color = vec4(ambient * 0.3f + diffuse + specular * 0.001f,1.0f);

}
