 #version 450								// use GLSL 4.5

layout (location = 0) in vec2 texture_coordinates;

layout (location = 0) out vec4 color;		//final output color (must have location)

layout(set = 1, binding = 0) uniform sampler2D texture_sampler;

void main() {

	color = texture(texture_sampler, texture_coordinates);
	//color = vec4(1.0f,0.0f,1.0f,1.0f);

}