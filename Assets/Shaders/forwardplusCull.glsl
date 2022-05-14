#version 430 core

#define TILE_SIZE 16
layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

struct PointLight {
	vec3 pos;
	float radius;
	vec4 colour;
};

struct TileAABB {
	vec4 min;
	vec4 max;
};

layout(std430, binding = 0) readonly buffer lightSSBO {
	PointLight pointLights[];
};


uniform uint noOfLights;

void main() {
	uint threadCount = TILE_SIZE * TILE_SIZE;
	uint batchCount = (noOfLights + threadCount - 1) / threadCount;
	uint tileNumber = ivec2(gl_NumWorkGroups.xy);
	for (unit i = 0; i < batchCount; i++) {
		uint lightIndex = i * threadCount * gl_LocalInvocationIndex;

		if (lightIndex >= noOfLights) {
			break;
		}

		vec4 position = pointLights[lightIndex].position;
		float radius = pointLights[lightIndex].radius;

	}
}