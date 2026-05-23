#version 450

const int MAX_LIGHTS = 16;

in vec3 f_position;
in vec2 f_uv;
in vec3 f_normal;

struct Light {
	vec3 position;
	vec3 color;
}

uniform Light lights[MAX_LIGHTS];

uniform sampler2D mainTexture;
uniform float Ka;//coeficiente de reflexão ambiente
uniform float Kd;//coeficiente de reflexão difusa
uniform float Ks;//coeficiente de reflexão especular
uniform float shininess;//expoente especular

out vec4 color;

void main() {
	color = texture(mainTexture, f_uv);
}
