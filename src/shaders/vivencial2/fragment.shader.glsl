#version 450

in vec3 f_position;
in vec3 f_normal;
in vec2 f_uv;

out vec4 color;

void main() {
	color = vec4(f_uv, 0.0, 1.0);
}
