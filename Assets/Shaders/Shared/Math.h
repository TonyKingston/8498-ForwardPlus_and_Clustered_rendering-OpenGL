#pragma once

const float PI = 3.141592653589793;
const float PI2 = PI * 2.0;

float remap(float minval, float maxval, float curval) {
	return (curval - minval) / (maxval - minval);
}

float remap(float val, float start1, float end1, float start2, float end2) {
	return (val - start1) / (end1 - start1) * (end2 - start2) + start2;
}

mat2 rotate2D(float r) {
	return mat2(cos(r), sin(r), -sin(r), cos(r));
}

float hash(float n) {
	return fract(sin(n) * 43758.5453);
}

// Copyright 2019 Google LLC.
// SPDX-License-Identifier: Apache-2.0

// Polynomial approximation in GLSL for the Turbo colormap
// Original LUT: https://gist.github.com/mikhailov-work/ee72ba4191942acecc03fe6da94fc73f

// Authors:
//   Colormap Design: Anton Mikhailov (mikhailov@google.com)
//   GLSL Approximation: Ruofei Du (ruofei@google.com)
vec3 turboColormap(float x) {
	const vec4 kRedVec4 = vec4(0.13572138, 4.61539260, -42.66032258, 132.13108234);
	const vec4 kGreenVec4 = vec4(0.09140261, 2.19418839, 4.84296658, -14.18503333);
	const vec4 kBlueVec4 = vec4(0.10667330, 12.64194608, -60.58204836, 110.36276771);
	const vec2 kRedVec2 = vec2(-152.94239396, 59.28637943);
	const vec2 kGreenVec2 = vec2(4.27729857, 2.82956604);
	const vec2 kBlueVec2 = vec2(-89.90310912, 27.34824973);

	x = clamp(x, 0, 1);
	vec4 v4 = vec4(1.0, x, x * x, x * x * x);
	vec2 v2 = v4.zw * v4.z;
	return vec3(
		dot(v4, kRedVec4) + dot(v2, kRedVec2),
		dot(v4, kGreenVec4) + dot(v2, kGreenVec2),
		dot(v4, kBlueVec4) + dot(v2, kBlueVec2)
	);
}
