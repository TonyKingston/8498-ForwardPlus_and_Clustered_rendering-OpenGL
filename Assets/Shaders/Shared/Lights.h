#pragma once

#ifdef __cplusplus
#include "GLSLTypeAliases.h"
#endif

struct PointLight {
    vec4 colour;
	vec4 pos;
	vec4 radius;
};