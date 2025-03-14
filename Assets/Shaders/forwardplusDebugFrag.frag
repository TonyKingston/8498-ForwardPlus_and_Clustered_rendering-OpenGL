#version 430 core

#define TILE_SIZE 16
#define MAX_LIGHTS_PER_TILE 16
#include "Shared/LightDefinitions.h"

 struct LightGrid {
	 uint count;
	 uint lightIndices[MAX_LIGHTS_PER_TILE];
 };


layout(std430, binding = 0) readonly buffer lightSSBO {
	PointLight pointLights[];
};

//layout(std430, binding = 2) buffer lightGridSSBO {
//	LightGrid lightGrid[];
//};

layout(std430, binding = 2) buffer lightGridSSBO {
	int lightIndices[];
};

//layout(std430, binding = 3) buffer globalLightIndexListSSBO {
//	uint globalLightIndexList[];
//};


//layout(std430, binding = 4) buffer globalIndexCountSSBO {
//	float testDepth[];
//};

uniform int noOfLights;
uniform int numTilesX;
uniform int tilePxX;

uniform vec3	cameraPos;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
//	vec4 shadowProj;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	float depth;
} IN;

out vec4 fragColor;

void main(void)
{
	float shadow = 1.0;
	
	/*if( IN . shadowProj . w > 0.0) {
		shadow = textureProj ( shadowTex , IN . shadowProj ) * 0.5f;
	}*/

	ivec2 tileID = ivec2(gl_FragCoord.xy) / ivec2(TILE_SIZE, TILE_SIZE); 
	int tileIndex = tileID.y * numTilesX + tileID.x;
	int lightCount = 0;
	//for (int i = 0; i < MAX_LIGHTS_PER_TILE && lightIndices[lightIndexOffset + i] != -1; i++; lightCount++);

	float shade = float(lightCount) / float(3);
	fragColor.rgb = vec3(shade);
	fragColor.a = 1.0;
}