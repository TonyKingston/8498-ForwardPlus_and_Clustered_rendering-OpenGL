#pragma once

#ifdef __cplusplus
#include "GLSLTypeAliases.h"
namespace NCL::GLSL {
#endif

struct PointLight {
	vec4 colour;
	vec4 pos;
	vec4 radius;
};

#ifdef __cplusplus
} // namespace
using PointLight = NCL::GLSL::PointLight;
#endif