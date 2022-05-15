#version 430 core

#define TILE_SIZE 16

uniform sampler2D 	mainTex;
uniform sampler2D   bumpTex;
//uniform sampler2DShadow shadowTex;

 struct PointLight {
	 vec4 colour;
	 vec4 pos;
	 vec4 radius;
 };

//struct PointLight {
//  vec3 pos;
//  float radius;
//  vec4 colour;
//};

struct LightGrid {
	uint offset;
	uint count;
};

layout(std430, binding = 0) readonly buffer lightSSBO {
	PointLight pointLights[];
};

layout(std430, binding = 2) buffer lightGridSSBO {
	LightGrid lightGrid[];
};

layout(std430, binding = 3) buffer globalLightIndexListSSBO {
	uint globalLightIndexList[];
};

uniform int noOfLights;
uniform int numTilesX;
uniform int tilePxX;

uniform vec3	cameraPos;

uniform bool hasTexture;
uniform bool hasBump;

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
	
    //uvec2 tiles = uvec2( gl_FragCoord.xy / tilePxX);
   //uint tileIndex = tiles.x + TILE_SIZE * tiles.y;
	
	uint lightCount = lightGrid[tileIndex].count;
    uint lightIndexOffset = lightGrid[tileIndex].offset;
//	lightIndexOffset = tileIndex * noOfLights;

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = IN.normal;
	if (hasBump) {
		normal = texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0;
		normal = normalize(TBN * normalize(normal));
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
	vec3 diffuseLight;
	vec3 specularLight;
	for (uint i = 0; i < lightCount; i++) {
//	for (int i = 0; i < noOfLights && globalLightIndexList[lightIndexOffset + i] != -1; i++
	    uint lightIndex = globalLightIndexList[lightIndexOffset + i];
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
			float sFactor = pow(rFactor, 60.0);

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
	// if (tileIndex == 0) {
	  // fragColor.rgb = vec3(1,0,0);
	// }
	// if (tileIndex == 2) {
	  // fragColor.rgb = vec3(0,1,0);
	// }
	// if (tileIndex == 20) {
	  // fragColor.rgb = vec3(0,1,0);
	// }
	// if (tileIndex == 500) {
 	   // if (lightGrid[tileIndex].count > 0) {
	    // fragColor.rgb = vec3(1,1,0);
	   // }
	// }
	// uint count = 0;
	// for (uint i = 0; i < 50; i++) {
	    // if (globalLightIndexList[lightIndexOffset + i] != -1) {
			// count++;
	    // }
	// }
	// float shade = float(count) / float(50 * 2); 
	// fragColor.rgb = vec3(shade);
	fragColor.a = 1.0;
}