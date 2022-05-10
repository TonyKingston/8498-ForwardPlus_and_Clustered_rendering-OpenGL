#version 330 core
uniform sampler2D diffuseTex;
uniform sampler2D diffuseLight;
uniform sampler2D specularLight;

uniform sampler2D skyboxTex;
uniform sampler2D scene_depth;

in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main (void) {
	vec3 diffuse = texture(diffuseTex, IN.texCoord).xyz;
	vec3 light = texture(diffuseLight, IN.texCoord).xyz;
	vec3 specular = texture(specularLight, IN.texCoord).xyz;

	fragColour.xyz = diffuse * 0.1; // ambient
	fragColour.xyz += diffuse * light; // lambert
	fragColour.xyz += specular; // Specular
	fragColour.a = 1.0;

	vec3 depth_tex = texture(scene_depth, IN.texCoord).xyz;

	if (depth_tex.rgb == vec3(1.0f,1.0f,1.0f)) {fragColour.rgb += texture(skyboxTex, IN.texCoord).xyz;}
        else if (depth_tex.rgb == vec3(0.2f,0.2f,0.2f)) {fragColour.rgb += texture(skyboxTex, IN.texCoord).xyz;}

	//fragColour.xy = IN.texCoord.xy;
	//fragColour.xy = IN.testingVec.xy;

//fragColour.y = 0;
//fragColour.w = 1.0f;
}
