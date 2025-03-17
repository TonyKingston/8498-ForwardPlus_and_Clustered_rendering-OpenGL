#pragma once

#ifdef __cplusplus
#include "GLSLTypeAliases.h"
namespace NCL::GLSL {
#endif

// Unused. AABBs are constructed in the culling shader. Might change at some point.
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

struct ClusterFrustum {
	Plane planes[4];
	vec4 nearFar;
};

// Unused
//struct LightGrid {
//	uint count;
//	uint lightIndices[MAX_LIGHTS_PER_TILE];
//};

#ifdef __cplusplus
} // namespace
#endif