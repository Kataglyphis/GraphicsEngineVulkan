#version 450								// use GLSL 4.5
#extension GL_KHR_vulkan_glsl : enable

layout(location=0) out vec3 frag_color;   // output color for vertex

// triangle vertex positions (will put into vertex buffer later)
vec3 positions[3] = vec3[](
			vec3(0.0, -0.4, 0.0),	
			vec3(0.4, 0.4, 0.0),
			vec3(-0.4, -0.4, 0.0)
);

vec3 colors[3] = vec3[](
						vec3(1.0f,0.0f,0.0f),
						vec3(0.0f,1.0f,0.0f),
						vec3(0.0f,0.0f,1.0f)
);

void main () {

	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
	frag_color = colors[gl_VertexIndex];

}