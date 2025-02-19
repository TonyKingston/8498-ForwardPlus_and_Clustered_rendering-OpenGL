#pragma once

const float PI = 3.141592653589793;
const float PI2 = PI * 2.0;

float remap(float minval, float maxval, float curval) {
	return (curval - minval) / (maxval - minval);
}

mat2 rotate2D(float r) {
	return mat2(cos(r), sin(r), -sin(r), cos(r));
}

float hash(float n) {
	return fract(sin(n) * 43758.5453);
}