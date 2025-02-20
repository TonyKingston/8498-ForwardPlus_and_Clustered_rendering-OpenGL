#version 430 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

layout(location = 0) in vec3 position;

struct PointLight {
    vec4 colour;
	vec4 pos;
	vec4 radius;
};

layout(std430, binding = 0) readonly buffer lightSSBO {
	PointLight pointLights[];
};

uniform int lightIndex;

void main (void) {
    PointLight light = pointLights[lightIndex];
	vec3 scale = vec3(light.radius.x);
	vec3 worldPos = (position * scale) + light.pos.xyz;
	gl_Position = (projMatrix * viewMatrix) * vec4(worldPos, 1.0);
}