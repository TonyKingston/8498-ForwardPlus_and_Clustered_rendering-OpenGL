#version 330 core

uniform sampler2D mainTex;
in Vertex {
	vec2 texCoord;
	} IN;

	out vec4 fragColour;
void main(void)	{
	vec2 inv = IN.texCoord;
	inv.y = 1-inv.y;
	fragColour = texture(mainTex, inv);
}