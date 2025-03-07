#version 430 core

#include "lighting.frag"

layout(binding = 0) uniform sampler2D depthTex;
layout(binding = 1) uniform sampler2D normTex;
layout(binding = 2) uniform sampler2D specTex;
//uniform sampler2D shadowTex;

uniform vec3 cameraPos;
uniform vec2 pixelSize; // reciprocal of resolution

layout(std430, binding = 0) readonly buffer lightSSBO {
	PointLight pointLights[];
};

uniform int lightIndex;
uniform mat4 inverseProjView;

out vec4 diffuseOutput;
out vec4 specularOutput;

void main (void) {
	vec2 texCoord = vec2 ( gl_FragCoord . xy * pixelSize );
	float depth = texture ( depthTex , texCoord . xy ).r;
	vec3 ndcPos = vec3 ( texCoord , depth ) * 2.0 - 1.0;
	vec4 invClipPos = inverseProjView * vec4 ( ndcPos , 1.0);
	vec3 worldPos = invClipPos . xyz / invClipPos.w;
	
	vec4 normalAndSpec = texture(normTex, texCoord.xy);
	float specSample = normalAndSpec.a;
	//vec3 normal = normalize ( texture ( normTex , texCoord.xy ).xyz *2.0 -1.0);
	vec3 normal = normalize(normalAndSpec.xyz * 2.0 - 1.0);
	vec3 viewDir = normalize ( cameraPos - worldPos );

	PointLight light = pointLights[lightIndex];
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);
	calculateLighting(light, worldPos, viewDir, normal, specSample, diffuse, specular);
	
//	float shadow = texture(shadowTex, texCoord.xy).r * 2.0 - 1.0;
	diffuseOutput = vec4 ( diffuse, 1.0);
	specularOutput = vec4 ( specular , 1.0);

//	diffuseOutput = vec4 ( attenuated * lambert * shadow , 1.0);
	//specularOutput = vec4 ( attenuated * specFactor * shadow * 0.33 , 1.0);
}
