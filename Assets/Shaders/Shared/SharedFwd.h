#pragma once

// Add forward declarations of shared types between C++ and glsl shaders here.
#ifdef __cplusplus
#include "GLSLTypeAliases.h"
namespace NCL::GLSL {
#endif

	struct PointLight;

#ifdef __cplusplus
} // namespace
using PointLight = NCL::GLSL::PointLight;
#endif