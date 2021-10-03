#version 450								// use GLSL 4.5

layout (location = 0) in vec3 frag_color;  // interpolated color from vertex (location must match)

layout (location = 0) out vec4 color;		//final output color (must have location)

void main() {

	color = vec4(frag_color, 1.0f);

}