#version 430 core

#include "Shared/TextureBindings.h"
#include "lighting.frag"

layout(binding = TEXTURE_BINDING_DIFFUSE) uniform sampler2D 	mainTex;
layout(binding = TEXTURE_BINDING_NORMAL) uniform sampler2D   bumpTex;
layout(binding = TEXTURE_BINDING_SPECULAR) uniform sampler2D   specTex;
//uniform sampler2DShadow shadowTex;
//
//struct PointLight {
//	vec4 colour;
//	vec4 pos;
//	float radius;
//};

// struct PointLight {
	// vec3 pos;
	// float radius;
	// vec4 colour;
// };

layout(std430, binding = 0) readonly buffer lightSSBO {
	PointLight pointLights[];
};

uniform int noOfLights;

uniform vec3	cameraPos;

uniform bool hasTexture;
uniform bool hasBump;
uniform bool hasSpec;

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

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = IN.normal;
	if (hasBump) {
		normal = texture(bumpTex, IN.texCoord).rgb * 2.0 - 1.0;
		normal = normalize(TBN * normalize(normal));
	}
	
	float specSample = 1;
	if (hasSpec) {
	    specSample = texture(specTex, IN.texCoord).r;
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
	for (int i = 0; i < noOfLights; i++) {
		PointLight light = pointLights[i];
		calculateLighting(light, IN.worldPos, viewDir, normal, specSample, diffuseLight, specularLight);
	}
	
	//fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2f));
	fragColor.rgb += albedo.rgb * diffuseLight;
	fragColor.rgb += specularLight.rgb;
	fragColor.a = 1.0;
}