#version 430 core

uniform sampler2D depthTex;
uniform sampler2D normTex;
//uniform sampler2D shadowTex;

uniform vec3 cameraPos;
uniform vec2 pixelSize; // reciprocal of resolution

struct PointLight {
	vec4 colour;
	vec4 pos;
	float radius;
};

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
	
	vec3 normal = normalize ( texture ( normTex , texCoord.xy ).xyz *2.0 -1.0);
	vec3 viewDir = normalize ( cameraPos - worldPos );

	PointLight light = pointLights[lightIndex];
	vec3 lightVec = light.pos.xyz - worldPos;
	float dist = length ( lightVec );
	float atten = 1.0 - clamp ( dist / light.radius , 0.0 , 1.0);

	if( atten == 0.0) {
	    discard;
	}
		
	vec3 incident = normalize ( lightVec);
	vec3 halfDir = normalize ( incident + viewDir );
	
	float lambert = clamp ( dot ( incident , normal ) ,0.0 ,1.0);
   	float rFactor = clamp ( dot ( halfDir , normal ) ,0.0 ,1.0);
	float specFactor = clamp ( dot ( halfDir , normal ) ,0.0 ,1.0);
	specFactor = pow ( specFactor , 60.0 );
	vec3 attenuated = light.colour.xyz * atten;
	
//	float shadow = texture(shadowTex, texCoord.xy).r * 2.0 - 1.0;
	diffuseOutput = vec4 ( attenuated * lambert, 1.0);
	specularOutput = vec4 ( attenuated * specFactor * 0.33 , 1.0);

//	diffuseOutput = vec4 ( attenuated * lambert * shadow , 1.0);
	//specularOutput = vec4 ( attenuated * specFactor * shadow * 0.33 , 1.0);
}
