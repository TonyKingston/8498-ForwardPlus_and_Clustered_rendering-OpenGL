#version 430 core

#define TILE_SIZE 16
layout(local_size_x = 1, local_size_y = 1) in;

// Using vec4 for better alignment on the GPU
struct TileAABB {
	vec4 min;
	vec4 max;
	vec4 extent;
};

layout(std430, binding = 1) buffer tileGrid {
	TileAABB tile[];
};

uniform mat4 inverseProj;
uniform vec2 pixelSize;
uniform int tilePxX;

uniform float near;
uniform float far;

vec4 screenToView(vec4 screenSpace);
vec3 AABBExtent(vec3 min, vec3 max);
vec3 ConstructPlane(vec3 A, vec3 B, float zDistance);

void main() {
	const vec3 eyePos = vec3(0, 0, 0);

	uint tileIndex = gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x;
	//uint tileIndex = gl_LocalInvocationIndex + gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z * gl_WorkGroupID.z;

	// Approximating the frustum of each tile with an AABB. A recommended optimization from Advancements in Tiled-Based
	// Compute Rendering by Gareth Thomas
	
	//vec3 viewSpace[8];
	//// screen space
	//viewSpace[0] = screenToView(vec4(gl_WorkGroupID.xy * TILE_SIZE, 1.0f, 1.0f)).xyz;
	//// Top right point, near
	//viewSpace[1] = screenToView(vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID..y) * TILE_SIZE, 1.0f, 1.0f)).xyz;
	//// Bottom left point, near
	//viewSpace[2] = screenToView(vec4(vec2(gl_WorkGroupID.x, gl_WorkGroupID.y + 1) * TILE_SIZE, 1.0f, 1.0f).xyz;
	//// Bottom right point, near
	//viewSpace[3] = screenToView(vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y + 1) * TILE_SIZE, 1.0f, 1.0f)).xyz;
	//
	//// Top left point, far
	//viewSpace[4] = screenToView(vec4(gl_WorkGroupID.xy * TILE_SIZE, -1.0, 1.0f)).xyz;
	//// Top right point, far
	//viewSpace[5] = screenToView(vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y) * TILE_SIZE, -1.0, 1.0f)).xyz;
	//// Bottom left point, far
	//viewSpace[6] = screenToView(vec4(vec2(gl_WorkGroupID.x, gl_WorkGroupID.y + 1) * TILE_SIZE, -1.0, 1.0f)).xyz;
	//// Bottom right point, far
	//viewSpace[7] = screenToView(vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y + 1) * TILE_SIZE, -1.0, 1.0f)).xyz;

	/*vec4 maxPoint = vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y + 1) * TILE_SIZE, -1.0, 1.0);
	vec4 minPoint = vec4(gl_WorkGroupID.xy * TILE_SIZE, -1.0, 1.0);*/

	vec4 maxPoint = vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y + 1) * tilePxX, -1.0, 1.0);
	vec4 minPoint = vec4(gl_WorkGroupID.xy * tilePxX, -1.0, 1.0);

	float tileNear = -near * pow(far / near, gl_WorkGroupID.z / float(gl_NumWorkGroups.z));
	float tileFar = -near * pow(far / near, (gl_WorkGroupID.z + 1) / float(gl_NumWorkGroups.z));
	
	// view space
	vec3 maxPointV = screenToView(maxPoint).xyz;
	vec3 minPointV = screenToView(minPoint).xyz;

	vec3 minPointNear = ConstructPlane(eyePos, minPointV, tileNear);
	vec3 minPointFar = ConstructPlane(eyePos, minPointV, tileFar);
	vec3 maxPointNear = ConstructPlane(eyePos, maxPointV, tileNear);
	vec3 maxPointFar = ConstructPlane(eyePos, maxPointV, tileFar);

	vec3 minPointAABB = min(min(minPointNear, minPointFar), min(maxPointNear, maxPointFar));
	vec3 maxPointAABB = max(max(minPointNear, minPointFar), max(maxPointNear, maxPointFar));

	tile[tileIndex].min = vec4(minPointAABB, 0.0);
	tile[tileIndex].max = vec4(maxPointAABB, 0.0);
	tile[tileIndex].extent = vec4(AABBExtent(minPointAABB, maxPointAABB), 0.0);

	//tile[tileIndex].min = vec4(1, 2, 3, 0.0);
	//tile[tileIndex].max = vec4(1, 2, 3, 0.0);
}

vec4 screenToView(vec4 screenSpace) {
	vec2 texCoord = screenSpace.xy * pixelSize;
	//vec4 clipSpace = vec4(texCoord * 2.0 - 1.0, screenSpace.z, screenSpace.w);
	vec4 clipSpace = vec4(vec2(texCoord.x, texCoord.y) * 2.0f - 1.0f, screenSpace.z, screenSpace.w);
	vec4 view = inverseProj * clipSpace;
	view = view / view.w;
	return view;
}

vec3 AABBExtent(vec3 min, vec3 max) {
  vec3 centre = (min + max) * 0.5;
  return abs(max - centre);
}

vec3 ConstructPlane(vec3 A, vec3 B, float zDistance) {

	vec3 normal = vec3(0.0, 0.0, 1.0);

	vec3 ab = B - A;

	//Computing the intersection length for the line and the plane
	float t = (zDistance - dot(normal, A)) / dot(normal, ab);

	//Computing the actual xyz position of the point along the line
	vec3 result = A + t * ab;

	return result;
}
