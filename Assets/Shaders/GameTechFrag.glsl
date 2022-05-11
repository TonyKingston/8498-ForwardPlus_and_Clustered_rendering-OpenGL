#version 430 core

uniform sampler2D 	mainTex;
uniform sampler2D   bumpTex;
//uniform sampler2DShadow shadowTex;

struct PointLight {
	vec3 pos;
	float radius;
	vec4 colour;
};

layout(std430, binding = 0) readonly buffer lightSSBO {
	PointLight pointLights[];
};

uniform int noOfLights;

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

	albedo.rgb = pow(albedo.rgb, vec3(2.2));

	fragColor.rgb = albedo.rgb * 0.005f; //ambient

	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	for (int i = 0; i < noOfLights; i++) {
		PointLight light = pointLights[i];
		vec3 lightVec = light.pos - IN.worldPos;
		vec3  incident = normalize(lightVec);
		float lambert = max(0.0, dot(incident, normal)) * 0.9;
		float distance = length(lightVec);
		float attenuation = 1.0f - clamp(distance / pointLights[i].radius, 0.0, 1.0);
		if (attenuation > 0.0f) {
			vec3 halfDir = normalize(incident + viewDir);

			float rFactor = max(0.0, dot(halfDir, normal));
			float sFactor = pow(rFactor, 60.0);

			fragColor.rgb += albedo.rgb * light.colour.rgb * lambert * attenuation; //diffuse light

			fragColor.rgb += light.colour.rgb * sFactor * attenuation; //specular light
		}
	}
	
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2f));
	fragColor.a = 1;

//fragColor.rgb = IN.normal;

	//fragColor = IN.colour;
	
	//fragColor.xy = IN.texCoord.xy;
	
	//fragColor = IN.colour;
}