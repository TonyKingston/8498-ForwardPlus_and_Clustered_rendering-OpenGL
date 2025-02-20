#version 400 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform mat4 viewMatrix 	= mat4(1.0f);
uniform mat4 projMatrix 	= mat4(1.0f);

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 tangent;

uniform mat4 inverseProjView;


uniform vec4 		objectColour = vec4(1,1,1,1);

uniform bool hasVertexColours = false;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 clipSpace;
	vec4 viewSpace;
} OUT;

void main(void)
{
	//mat4 mvp 		  = (projMatrix * viewMatrix * modelMatrix);
	mat3 normalMatrix = transpose ( inverse ( mat3 ( modelMatrix )));

	vec3 wNormal    = normalize ( normalMatrix * normalize ( normal ));
	vec3 wTangent   = normalize(normalMatrix * normalize(tangent.xyz));
	
	OUT.worldPos 	= ( modelMatrix * vec4 ( position.xyz ,1)).xyz;
	OUT.normal 		= wNormal;
	OUT.tangent     = wTangent;
	OUT.binormal    = cross(wTangent, wNormal) * tangent.w;
	OUT.texCoord	= texCoord;
	OUT.colour		= objectColour;

	if(hasVertexColours) {
		OUT.colour		= objectColour * colour;
	}
	vec4 view = viewMatrix * modelMatrix * vec4(position.xyz, 1.0);
 	vec4 clip = projMatrix * view;
	gl_Position = clip;
	OUT.clipSpace = gl_Position;
	OUT.viewSpace = view;
}