#version 450

const int MAX_LIGHTS = 16;

const float kc = 1.0;
const float kl = 0.09;
const float kq = 0.032;

in vec3 position;
in vec2 uv;
in vec3 normal;

struct Light {
	vec3 position;
	vec3 color;
	bool enabled;
};

uniform Light lights[MAX_LIGHTS];
uniform int lightsCount;

uniform sampler2D mainTexture;
uniform float ka; //coeficiente de reflexão ambiente
uniform float kd; //coeficiente de reflexão difusa
uniform float ks; //coeficiente de reflexão especular
uniform float shininess; //expoente especular

out vec4 color;

void main() {
	vec3 albedo = texture(mainTexture, uv).xyz;

	vec3 ambient = vec3(0);
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);

	for (int i = 0; i < lightsCount; i++) {
		Light light = lights[i];
		
		if (!light.enabled) continue;

		vec3 offset = light.position - position;

		vec3 lightDirection = normalize(offset);
		vec3 viewDirection = normalize(-position);
		vec3 reflectDirection = normalize(reflect(-lightDirection, normal));

		float lightDistance = length(offset);
		float attenuation = 1.0 / (kc + kl * lightDistance + kq * lightDistance * lightDistance);

		float diff = max(dot(normal, lightDirection), 0.0);
		float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), shininess);

		diffuse += kd * diff * light.color * attenuation;
		specular += ks * spec * light.color * attenuation;
	}

	vec3 result = (ambient + diffuse) * albedo + specular;

	color = vec4(result, 1.0);
}
