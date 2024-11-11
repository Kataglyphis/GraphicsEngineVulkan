#version 460

in vec2 tex_coords;

out vec4 color;

uniform sampler2D loading_screen;

void main() {

	color = vec4(texture(loading_screen, tex_coords).rgb, 1.0f);

}