#version 450

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_uv;

uniform mat4 model;

out vec3 f_position;
out vec3 f_normal;
out vec2 f_uv;

void main() {
    f_position = v_position;
    f_normal = v_normal;
    f_uv = v_uv;

	gl_Position = model * vec4(v_position, 1.0);
}