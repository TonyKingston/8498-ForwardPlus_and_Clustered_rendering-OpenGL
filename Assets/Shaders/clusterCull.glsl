#version 430 core

#define TILE_SIZE 16
#define MAX_LIGHTS_PER_TILE 64
layout(local_size_x = 16, local_size_y = 8, local_size_z = 4) in;
//layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
//layout(local_size x = 32) in;

//uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform sampler2D depthTex;

 struct PointLight {
	 vec4 colour;
	 vec4 pos;
	 vec4 radius;
 };

//struct PointLight {
//  vec3 pos;
//  float radius;
//  vec4 colour;
//};

struct TileAABB {
	vec4 min;
	vec4 max;
	vec4 extent;
};

struct Plane {
	vec4 normal;
	vec4 distance;
};

struct Frustum {
	Plane planes[4];
};

//struct LightGrid {
//	uint offset;
//	uint count;
//};

//struct LightGrid {
//	uint count;
//	uint lightIndices[MAX_LIGHTS_PER_TILE];
//};
//
//struct ViewFrustum {
//	vec4 planes[6];
//	vec4 points[8]; // 0-3 near 4-7 far
//};

layout(std430, binding = 0) readonly buffer lightSSBO {
	PointLight pointLights[];
};

//layout(std430, binding = 1) buffer gridSSBO {
//	TileAABB tiles[];
//};

layout(std430, binding = 1) buffer tileGrid {
	Frustum tile[];
};


//layout(std430, binding = 2) buffer lightGridSSBO {
//	LightGrid lightVisibilities[];
//};

layout(std430, binding = 2) buffer lightGridSSBO {
	int lightIndices[];
};

//layout(std430, binding = 3) buffer globalLightIndexListSSBO {
//	uint globalLightIndexList[];
//};

layout(std430, binding = 4) buffer globalIndexCountSSBO {
	float testDepth[];
};

uniform int noOfLights;
uniform uint totalNumLights;
uniform ivec2 screenSize;
uniform vec2 pixelSize;
uniform mat4 invProj;

//shared PointLight sharedLights[TILE_SIZE * TILE_SIZE];
//shared uint globalIndexLightCount;

//shared vec4 frustumPlanes[6];
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
shared int visibleLightIndices[MAX_LIGHTS_PER_TILE];
shared Frustum tileFrustum;

shared mat4 viewProjMatrix;
shared mat4 invViewProj;
//shared mat4 invProj;

//bool sphereAABBIntersect(uint light, uint tile);
//float sqDistPointAABB(vec3 point, uint tile);
//bool quickIntersect(uint light, uint tile);
//bool frustumSphereIntersect(uint light, uint tile);
TileAABB AABBtransform(TileAABB aabb, mat4 mat);
vec4 ClipToView(vec4 clip);
bool SphereInsideFrustum(vec3 posVs, float radius, Frustum frustum, float zNear, float zFar);
bool SphereInsidePlane(vec3 posVs, float radius, Plane plane);

void main() {
	ivec2 tileId = ivec2(gl_WorkGroupID.xy);
	ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);
	ivec2 location = ivec2(gl_GlobalInvocationID.xy);

	uint tileIndex = tileId.y * tileNumber.x + tileId.x;

	if (gl_LocalInvocationIndex == 0) {
		vec2 test = vec2(location) * pixelSize;
		testDepth[tileIndex] = test.y;
	}
	//vec2 texCoord = (vec2(location)) / screenSize;
	//vec2 texCoord = vec2(location) / ivec2(1264, 681);

	vec2 texCoord = vec2(location) * pixelSize;
	float depth = texture(depthTex, texCoord).r;
	// Convert sampled depth into viewspace -1 to 1 range
//	depth = depth * 2.0 - 1.0;
	//depth = (0.5 * projMatrix[3][2]) / (depth + 0.5 * projMatrix[2][2] - 0.5);

	uint depthInt = floatBitsToUint(depth);

    // One thread to init values and get the group's tile frustum.
	// Could also reset everything in the indices buffer to 0 if so desired
	if (gl_LocalInvocationIndex == 0) {
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0;
		visibleLightCount = 0;
		viewProjMatrix = projMatrix * viewMatrix;
		//invProj = inverse(projMatrix);
		invViewProj = inverse(viewProjMatrix);
		tileFrustum = tile[tileIndex];

		//if (tileId.x < 78) {
		//	tileFrustum.planes[1] = tile[tileIndex + 1].planes[0];
		//	tileFrustum.planes[1].normal = -tileFrustum.planes[1].normal;
		//}
		//if (tileId.y < 42) {
		//	tileFrustum.planes[2] = tile[tileIndex + (tileId.y * tileNumber.x)].planes[3];
		//	tileFrustum.planes[2].normal = -tileFrustum.planes[2].normal;
		//}

	} 
	
	barrier();

	atomicMin(minDepthInt, depthInt);
	atomicMax(maxDepthInt, depthInt);

	barrier();

	float minDepth = uintBitsToFloat(minDepthInt);
	minDepth = minDepth * 2.0f - 1.0f;
	float maxDepth = uintBitsToFloat(maxDepthInt);
	maxDepth = maxDepth * 2.0f - 1.0f;


	float minDepthVS = ClipToView(vec4(0.0, 0.0, minDepth, 1.0)).z;
	float maxDepthVS = ClipToView(vec4(0.0, 0.0, maxDepth, 1.0)).z;
	float nearClipVS = ClipToView(vec4(0.0, 0.0, 0.0, 1.0)).z;

	Plane minPlane = { vec4(0.0, 0.0, -1.0, 0.0), vec4(-minDepthVS, 0.0,0.0,0.0) };

	barrier();

	// Each batch will handle 256 lights with a tile size of 16
	uint threadCount = TILE_SIZE * TILE_SIZE;
	uint batchCount = (noOfLights + threadCount - 1) / threadCount;
	for (uint i = 0; i < batchCount; ++i) {
		uint lightIndex = i * threadCount + gl_LocalInvocationIndex;

		if (lightIndex >= noOfLights) {
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
				uint offset = atomicAdd(visibleLightCount, 1);
				visibleLightIndices[offset] = int(lightIndex);
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

/*void main() {
	globalIndexLightCount = 0;
	//uint threadCount = TILE_SIZE * TILE_SIZE;
	uint threadCount = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
	uint batchCount = (noOfLights + threadCount - 1) / threadCount;
	ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);

//	uint tileIndex = gl_WorkGroupID.y * tileNumber.x + gl_WorkGroupID.x;
	//uint tileIndex = gl_LocalInvocationIndex + gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z * gl_WorkGroupID.z;
	uint tileIndex = gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x;
	
	//if (gl_LocalInvocationIndex == 0) {
		// uint offset = tileIndex * 50;
		// for (uint i = 0; i < 50; i++) {
			// globalLightIndexList[offset + i] = -1;
		// }
	// }
	//barrier();

	uint visibleLightCount = 0;
	uint visibleLightIndices[100];

	for (uint i = 0; i < batchCount; ++i) {
		uint lightIndex = i * threadCount + gl_LocalInvocationIndex;

		if (lightIndex >= noOfLights) {
			break;
		}

	//	lightIndex = min(lightIndex, noOfLights);

		sharedLights[gl_LocalInvocationIndex] = pointLights[lightIndex];
		barrier();

		for (uint light = 0; light < threadCount; ++light) {
			if (sphereAABBIntersect(light, tileIndex)) {
		//	if (quickIntersect(light, tileIndex)) {
		//	if (frustSphereIntersect(light, tileIndex)) {
				visibleLightIndices[visibleLightCount] = i * threadCount + light;
				visibleLightCount += 1;
			}
		}
	}

	barrier();

	uint offset = atomicAdd(globalIndexLightCount, visibleLightCount);

	for (uint i = 0; i < visibleLightCount; ++i) {
		globalLightIndexList[offset + i] = visibleLightIndices[i];
	}

	lightGrid[tileIndex].offset = offset;
	lightGrid[tileIndex].count = visibleLightCount;

	//tiles[tileIndex].min = vec4(tileIndex, 0.0, 0.0, 0.0);
}*/

// We know 16 is length. Max - min. Pythagoras?
//bool sphereAABBIntersect(uint light, uint tile) {
//	float radius = sharedLights[light].radius;
//	vec3 center = vec3(viewMatrix * sharedLights[light].position);
//	float distSq = sqDistPointAABB(center, tile);
//
//	return distSq <= (radius * radius);
//}

//bool frustumSphereIntersect(uint light, uint tile) {
//	//vec3 center = vec3(viewMatrix * sharedLights[light].pos);
//	//float radius = sharedLights[light].radius.x;
//	float distance = 0.0;
//	//for (uint j = 0; j < 6; j++) {
//	//	distance = dot(position, frustumPlanes[j]) + radius;
//
//	//	// If one of the tests fails, then there is no intersection
//	//	if (distance <= 0.0) {
//	//		break;
//	//	}
//	//}
//	return false;
//}
//
//bool quickIntersect(uint light, uint tile) {
//	TileAABB currentTile = tiles[tile];
//	vec4 tileCenter = (currentTile.min + currentTile.max) * 0.5;
//	//vec3 center = vec3(viewMatrix * vec4(sharedLights[light].pos, 0));
//	vec3 center = vec3(viewMatrix * sharedLights[light].pos);
//	float radius = sharedLights[light].radius.x;
//	vec3 vDelta = max(vec3(0,0,0), abs(tileCenter.xyz - center) - currentTile.extent.xyz);
//	float sqDist = dot(vDelta, vDelta);
//	return sqDist <= (radius * radius);
//}
//
//bool sphereAABBIntersect(uint light, uint tile) {
//	vec3 center = vec3(viewMatrix * sharedLights[light].pos);
//	float radius = sharedLights[light].radius.x;
//	//float radius = pointLights[light].radius.x;
//	//vec3 center = vec3(viewMatrix * pointLights[light].pos);
//	float sqDist = sqDistPointAABB(center, tile);
//
//	return sqDist <= (radius * radius);
//}

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

//float sqDistPointAABB(vec3 point, uint tile) {
//	float sqDist = 0.0;
//	//TileAABB currentTile = tiles[tile];
//	tiles[tile].max[3] = tile;
//	for (int i = 0; i < 3; ++i) {
//		float v = point[i];
//		if (v < currentTile.min[i]) {
//			sqDist += (currentTile.min[i] - v) * (currentTile.min[i] - v);
//		}
//		if (v > currentTile.max[i]) {
//			sqDist += (v - currentTile.max[i]) * (v - currentTile.max[i]);
//		}
//	}
//	return sqDist;
//}

TileAABB AABBtransform(TileAABB aabb, mat4 mat) {
	vec3 _min = aabb.min.xyz;
	vec3 _max = aabb.max.xyz;
	vec3 corners[8];
	corners[0] = _min;
	corners[1] = vec3(_min.x, _max.y, _min.z);
	corners[2] = vec3(_min.x, _max.y, _max.z);
	corners[3] = vec3(_min.x, _min.y, _max.z);
	corners[4] = vec3(_max.x, _min.y, _min.z);
	corners[5] = vec3(_max.x, _max.y, _min.z);
	corners[6] = _max;
	corners[7] = vec3(_max.x, _min.y, _max.z);
	_min = vec3(1000000);
	_max = vec3(-1000000);

	for (uint i = 0; i < 8; ++i) {
		corners[i] = (mat * vec4(corners[i], 1)).xyz;
		_min = min(_min, corners[i]);
		_max = max(_max, corners[i]);
	}

	TileAABB _aabb;
	_aabb.min = vec4(_min, 0.0);
	_aabb.max = vec4(_max, 0.0);
	_aabb.extent = vec4(0.0);
	return aabb;
}