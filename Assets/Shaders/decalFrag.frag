#version 400 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform mat4 viewMatrix 	= mat4(1.0f);
uniform mat4 projMatrix 	= mat4(1.0f);

uniform sampler2D depthTex; // Depth texture of first pass
uniform sampler2D mainTex; // Diffuse texture map
uniform sampler2D normalTex;


uniform vec3	cameraPos;
uniform vec2 pixelSize; // reciprocal of resolution
uniform mat4 inverseProjView;
uniform float aspect;
uniform mat4 inverseModel;

uniform bool hasTexture;
uniform bool hasBump;

uniform float far;


in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 clipSpace;
	vec4 viewSpace;
} IN ;

out vec4 fragColour;
out vec4 paintColour;

vec4 ReconstructPos(float z, vec2 uv) {
  vec4 wPos = vec4(uv * 2.0 - 1.0, z, 1.0);
  wPos = inverseProjView * wPos;
  return wPos / wPos.w;
}

void main ( void ) {
   vec3 screenPos = (IN.clipSpace.xyz / IN.clipSpace.w);

//Convert into a texture coordinate
//  vec2 texCoord = vec2(
//    (1 + screenPos.x) / 2 + (0.5 / pixelSize.x),
//    (1 - screenPos.y) / 2 + (0.5 / pixelSize.y)
//	);
   // Texture uses NEAREST for filtering so we don't need to add 0/5 / pixelsize
    vec2 texCoord = screenPos.xy / 2 + 0.5;
	float depth = texture ( depthTex , texCoord.xy).r * 2.0 - 1.0;

	vec3 ndcSample = vec3(screenPos.xy, depth);
	vec4 hViewPos = inverse(projMatrix) * vec4(ndcSample, 1.0); 

	//vec3 viewRay = IN.viewSpace.xyz * (far / -IN.viewSpace.z);

	vec3 viewPos = hViewPos.xyz / hViewPos.w;
	//viewPos = viewRay * depth;
	vec3 worldPos = (inverse(viewMatrix) * vec4(viewPos, 1.0)).xyz;
	vec3 objectPosition = (inverseModel * vec4(worldPos, 1.0)).xyz;

	if (abs(objectPosition.x) > 0.5) discard;
	if (abs(objectPosition.y) > 0.5) discard;
	if (abs(objectPosition.z) > 0.5) discard;

	vec3 ddxWp = dFdx(objectPosition);
	vec3 ddyWp = dFdy(objectPosition);
	vec3 normal = normalize(cross(ddyWp, ddxWp));

	vec3 binormal = normalize(ddxWp);
	vec3 tangent = normalize(ddyWp);
	mat3 TBN = mat3(IN.tangent, IN.binormal, IN.normal);
	normal = TBN * normal;


	// Make decal fade out
	/*float distance = abs(objectPosition.z);
	float scaleDistance = max(distance * 2.0, 1.0);
	float fadeOut = 1.0 - scaleDistance;*/
	vec3 worldNormal = texture2D(normalTex, texCoord.xy).xyz;
	vec3 decalNormal = normalize((inverseModel * vec4(worldNormal.xyz, 0.0))).xyz;
	vec2 decalTexCoord = objectPosition.xz;
	decalTexCoord -= objectPosition.y * decalNormal.xz;
	vec2 decalTexCoordZ = (objectPosition.xz) + 0.5;
	vec2 decalTexCoordY = (objectPosition.xy) + 0.5;

	vec4 colour;

	colour = texture(mainTex, decalTexCoordZ);
	
	fragColour.xyz = colour.xyz * IN.colour.xyz;
	fragColour.a = 1.0f;

	if(fragColour.xyz == vec3(0,0,0)){
		discard;
	}

}
