#pragma once

// Add forward declarations of shared types between C++ and glsl shaders here.
#ifdef __cplusplus
#include "GLSLTypeAliases.h"
namespace NCL::GLSL {
#endif

	struct PointLight;
	struct Plane;
	struct Frustum;
	struct ClusterFrustum;

#ifdef __cplusplus
} // namespace
using PointLight = NCL::GLSL::PointLight;
using TilePlane = NCL::GLSL::Plane;
using TileFrustum = NCL::GLSL::Frustum;
using ClusterFrustum = NCL::GLSL::ClusterFrustum;

#endif