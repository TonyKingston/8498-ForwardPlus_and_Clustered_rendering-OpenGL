#version 430 core

#define TILE_SIZE 16
layout(local_size_x = 1, local_size_y = 1) in;

struct TileAABB {
	vec4 min;
	vec4 max;
};

layout(std430, binding = 1) buffer tileGrid {
	TileAABB tile[];
};

uniform mat4 inverseProj;
uniform vec2 pixelSize;

vec4 screenToView(vec4 screenSpace);

void main() {
	const vec3 eyePos = vec3(0, 0, 0);

	uint tileIndex = gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x;
	
	// screen space
	vec4 maxPoint = vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y + 1) * TILE_SIZE, -1.0, 1.0);
	vec4 minPoint = vec4(gl_WorkGroupID.xy * TILE_SIZE, -1.0, 1.0);

	// view space
	vec3 maxPointV = screenToView(maxPoint).xyz;
	vec3 minPointV = screenToView(minPoint).xyz;

	tile[tileIndex].min = vec4(minPointV, 0.0);
	tile[tileIndex].max = vec4(maxPointV, 0.0);
}

vec4 screenToView(vec4 screenSpace) {
	vec2 texCoord = screenSpace.xy * pixelSize;
	vec4 clipSpace = vec4(texCoord * 2.0 - 1.0, screenSpace.z, screenSpace.w);
	vec4 view = inverseProj * clipSpace;
	view = view / view.w;
	return view;
}