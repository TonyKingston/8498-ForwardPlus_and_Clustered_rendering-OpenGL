#version 430 core

#define THREADS 32
#define MAX_LIGHTS_PER_TILE 2048
#include "Shared/ComputeBindings.h"

layout(local_size_x = THREADS, local_size_y = 1, local_size_z = 1) in;

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

struct PointLight {
	vec4 colour;
	vec4 pos;
	vec4 radius;
};

struct Plane {
	vec4 normal;
	vec4 distance;
};

struct Frustum {
	Plane planes[4];
	vec4 nearFar;
};

layout(std430, binding = COMPUTE_BINDING_LIGHT_BUFFER) readonly buffer lightSSBO {
	PointLight pointLights[];
};

layout(std430, binding = COMPUTE_BINDING_GRID_BUFFER) buffer tileGrid {
	Frustum tile[];
};

layout(std430, binding = COMPUTE_BINDING_LIGHT_INDEX_BUFFER) buffer lightGridSSBO {
	int lightIndices[];
};

layout(std430, binding = COMPUTE_BINDING_ACTIVE_CLUSTERS_BUFFER) buffer activeClusterSSBO {
	int activeClusters[];
};

layout(std430, binding = COMPUTE_BINDING_TEST3) buffer countSSBO {
	uint activeClusterCount;
};

uniform int noOfLights;
uniform uint totalNumLights;
uniform ivec2 screenSize;
uniform vec2 pixelSize;
uniform mat4 invProj;
uniform float near;
uniform float far;

shared uint visibleLightCount;
shared int visibleLightIndices[MAX_LIGHTS_PER_TILE];
shared Frustum tileFrustum;
shared bool clusterActive;
shared mat4 viewProjMatrix;

vec4 ClipToView(vec4 clip);
bool SphereInsideFrustum(vec3 posVs, float radius, Frustum frustum, float zNear, float zFar);
bool SphereInsidePlane(vec3 posVs, float radius, Plane plane);

void main() {
	ivec2 tileId = ivec2(gl_WorkGroupID.xy);
	ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);
	ivec2 location = ivec2(gl_GlobalInvocationID.xy);

	uint tileIndex = gl_WorkGroupID.x +
		gl_WorkGroupID.y * gl_NumWorkGroups.x +
		gl_WorkGroupID.z * (gl_NumWorkGroups.x * gl_NumWorkGroups.y);

	if (gl_LocalInvocationIndex == 0) {
		visibleLightCount = 0;
		viewProjMatrix = projMatrix * viewMatrix;
		tileFrustum = tile[tileIndex];
		clusterActive = false;

		if (activeClusters[tileIndex] == 1) {
			clusterActive = true;
		}
	}

	barrier();

	float minDepthVS = tileFrustum.nearFar.x;
	float maxDepthVS = tileFrustum.nearFar.y;
	float nearClipVS = ClipToView(vec4(0.0, 0.0, 0.0, 1.0)).z;

	Plane minPlane = { vec4(0.0, 0.0, -1.0, 0.0), vec4(-minDepthVS, 0.0,0.0,0.0) };

	barrier();

	//uint threadCount = TILE_SIZE * TILE_SIZE;
	uint threadCount = THREADS;
	uint batchCount = (noOfLights + threadCount - 1) / threadCount;
	bool isClusterActive = clusterActive;
	uint loopCount = (isClusterActive) ? batchCount : 0;
	for (uint i = 0; i < loopCount; ++i) {
		uint lightIndex = i * threadCount + gl_LocalInvocationIndex;

		if (lightIndex >= noOfLights || visibleLightCount >= MAX_LIGHTS_PER_TILE) {
			break;
		}

		/*lightIndex = min(lightIndex, noOfLights);*/
		PointLight light = pointLights[lightIndex];
		vec4 position = light.pos;
		position.w = 1.0;
		float radius = light.radius.x;
		vec4 vPos = viewMatrix * position;

		if (SphereInsideFrustum(vPos.xyz, radius, tileFrustum, nearClipVS, maxDepthVS)) {
			if (!SphereInsidePlane(vPos.xyz, radius, minPlane)) {
				if (visibleLightCount < MAX_LIGHTS_PER_TILE) {
					uint offset = atomicAdd(visibleLightCount, 1);
					visibleLightIndices[offset] = int(lightIndex);
				}
			}
		}
	}

	barrier();

	if (gl_LocalInvocationIndex == 0) {
		uint offset = tileIndex * MAX_LIGHTS_PER_TILE;

		for (uint i = 0; i < visibleLightCount && i < MAX_LIGHTS_PER_TILE; i++) {
			lightIndices[offset + i] = visibleLightIndices[i];
		}

		if (visibleLightCount < MAX_LIGHTS_PER_TILE) {
			lightIndices[offset + visibleLightCount] = -1;
		}
	}
}

vec4 ClipToView(vec4 clip) {
	vec4 view = invProj * clip;
	view = view / view.w;
	return view;
}

bool SphereInsideFrustum(vec3 posVs, float radius, Frustum frustum, float zNear, float zFar) {
	bool result = true;

	if (posVs.z - radius > zNear || posVs.z + radius < zFar) {
		result = false;
	}

	for (int i = 0; i < 4 && result; i++) {
		if (SphereInsidePlane(posVs, radius, frustum.planes[i])) {
			result = false;
		}
	}

	return result;
}

bool SphereInsidePlane(vec3 posVs, float radius, Plane plane) {
	return (dot(plane.normal.xyz, posVs) - plane.distance.x) < -radius;
}