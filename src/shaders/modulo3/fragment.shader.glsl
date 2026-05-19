#version 450

in vec3 f_position;
in vec2 f_uv;
in vec3 f_normal;

uniform sampler2D mainTexture;

out vec4 color;

void main() {
	color = texture(mainTexture, f_uv);
}
