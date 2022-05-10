#version 400 core

uniform sampler2D 	mainTex;
uniform sampler2DShadow shadowTex;
uniform sampler2D   bumpTex;


uniform vec3	lightPos;
uniform float	lightRadius;
uniform vec4	lightColour;

uniform vec3	cameraPos;

uniform bool hasTexture;
uniform bool hasBump;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;	
	vec3 worldPos;
	float depth;
} IN;

out vec4 fragColor;

void main(void)
{
	vec2 inverse = IN.texCoord;
	inverse.y = 1- inverse.y;
	float shadow = 1.0; // New !
	vec3 bumpNormal;
	if( IN . shadowProj . w > 0.0) { // New !
		shadow = textureProj ( shadowTex , IN . shadowProj ) * 0.5f;
	}

	vec3  incident = normalize ( lightPos - IN.worldPos );

	mat3 TBN = mat3 ( normalize ( IN.tangent ), 
       normalize ( IN.binormal ) , normalize( IN.normal ));

	vec3 viewDir = normalize ( cameraPos - IN.worldPos );
	vec3 halfDir = normalize ( incident + viewDir );

	float lambert;
	float rFactor;
	if (hasBump) {
	 bumpNormal = texture ( bumpTex , inverse).rgb;
	 bumpNormal = normalize ( TBN * normalize ( bumpNormal * 2.0 - 1.0));
	 lambert = max (0.0 , dot ( incident , bumpNormal )) * 0.9;
	 rFactor = clamp(dot ( halfDir , bumpNormal ), 0.0, 1.0);
	} else {
	 lambert  = max (0.0 , dot ( incident , IN.normal )) * 0.9; 
	 rFactor = max (0.0 , dot ( halfDir , IN.normal ));

	}

	float sFactor = pow ( rFactor , 160.0 );
	
	vec4 albedo = IN.colour;
	
	if(hasTexture) {
	
	 albedo *= texture(mainTex, inverse);
	}

	
	albedo.rgb = pow(albedo.rgb, vec3(2.2));
	
	fragColor.rgb = albedo.rgb * 0.05f; //ambient
	
	fragColor.rgb += albedo.rgb * lightColour.rgb * lambert * shadow; //diffuse light
	
	fragColor.rgb += lightColour.rgb * sFactor * shadow; //specular light
	
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2f));
	
	fragColor.a = albedo.a;
}