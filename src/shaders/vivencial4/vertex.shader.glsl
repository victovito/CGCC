#version 450

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_uv;
layout (location = 2) in vec3 v_normal;

uniform mat4 projection;
uniform mat4 model;

out vec3 position;
out vec2 uv;
out vec3 normal;

void main() {
    position = (model * vec4(v_position, 1.0)).xyz;
    normal = normalize(mat3(transpose(inverse(model))) * v_normal);
    uv = v_uv;

	gl_Position = projection * model * vec4(v_position, 1.0);
}