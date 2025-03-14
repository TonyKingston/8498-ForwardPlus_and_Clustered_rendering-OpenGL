#version 430 core
#include "Shared/LightDefinitions.h"

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

layout(location = 0) in vec3 position;

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