#version 430 core

#define TILE_SIZE 16
#define MAX_LIGHTS_PER_TILE 2048

#include "Shared/Debug.h"
#include "Shared/TextureBindings.h"

layout(binding = TEXTURE_BINDING_DIFFUSE) uniform sampler2D 	mainTex;
layout(binding = TEXTURE_BINDING_NORMAL) uniform sampler2D   bumpTex;
layout(binding = TEXTURE_BINDING_SPECULAR) uniform sampler2D   specTex;
//uniform sampler2DShadow shadowTex;

 struct PointLight {
	 vec4 colour;
	 vec4 pos;
	 vec4 radius;
 };

 struct LightGrid {
	 uint count;
	 uint lightIndices[MAX_LIGHTS_PER_TILE];
 };


layout(std430, binding = 0) readonly buffer lightSSBO {
	PointLight pointLights[];
};

layout(std430, binding = 2) buffer lightGridSSBO {
	int lightIndices[];
};

//layout(std430, binding = 4) buffer globalIndexCountSSBO {
//	float testDepth[];
//};

uniform int noOfLights;
uniform int numTilesX;
uniform int tilePxX;

uniform vec3	cameraPos;

uniform bool hasTexture;
uniform bool hasBump;
uniform bool hasSpec;
uniform bool inDebug;

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
	
	uint lightCount = 0;
	uint lightIndexOffset = tileIndex * MAX_LIGHTS_PER_TILE;

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = IN.normal;
//	normal = normalize(TBN * normalize(normal));

	if (hasBump) {
		normal = texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0;
		normal = normalize(TBN * normalize(normal));
	}
	
	float specSample = 1;
	if (hasSpec) {
	    specSample = texture2D(specTex, IN.texCoord).r;
	}

	vec4 albedo = IN.colour;
	if (hasTexture) {
		albedo *= texture(mainTex, IN.texCoord);
	}

	if (albedo.a < 0.1) {
		discard;
	}

	//albedo.rgb = pow(albedo.rgb, vec3(2.2));

	fragColor.rgb = albedo.rgb * 0.1f; //ambient

	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 diffuseLight = vec3(0);
	vec3 specularLight = vec3(0);
//	for (uint i = 0; i < lightCount; i++) {
	for (int i = 0; i < MAX_LIGHTS_PER_TILE && lightIndices[lightIndexOffset + i] != -1; i++) {
		lightCount++;
	//	uint lightIndex = globalLightIndexList[lightIndexOffset + i];
		uint lightIndex = lightIndices[lightIndexOffset + i];
		PointLight light = pointLights[lightIndex];
		vec3 lightVec = light.pos.xyz - IN.worldPos;
		//vec3 lightVec = light.pos - IN.worldPos;
		//float lambert = max(0.0, dot(incident, normal)) * 0.9;

		float distance = length(lightVec);
		float attenuation = 1.0f - clamp(distance / light.radius.x, 0.0, 1.0);
		
		if (attenuation > 0.0f) {
			vec3  incident = normalize(lightVec);
			vec3 halfDir = normalize(incident + viewDir);

			float lambert = clamp(dot(incident, normal), 0.0, 1.0);

			//float rFactor = max(0.0, dot(halfDir, normal));
			float rFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
			float sFactor = pow(rFactor, 60.0) * specSample;

			vec3 attenuated = light.colour.rgb * attenuation;
			//fragColor.rgb += albedo.rgb * attenuated * lambert; //diffuse light
			//fragColor.rgb += albedo.rgb * attenuated * lambert;
			//fragColor.rgb += light.colour.rgb * attenuated * sFactor * 0.33;
			diffuseLight.rgb += attenuated * lambert; //diffuse light
			specularLight.rgb += attenuated * sFactor * 0.33; //specular light
		}
	}
	
	//fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2f));
	fragColor.rgb += albedo.rgb * diffuseLight;
	fragColor.rgb += specularLight.rgb;

	//fragColor.rgb = vec3(0.1, 0.1, 0.2);

	if (inDebug) {
		fragColor.rgb = getDebugColour(lightCount);
	}

	fragColor.a = 1.0;
}