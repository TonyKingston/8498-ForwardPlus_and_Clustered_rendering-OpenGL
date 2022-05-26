#version 430 core

uniform sampler2D 	mainTex;
uniform bool hasMask;

in Vertex
{
	vec2 texCoord;
} IN;

void main(void)
{
    if (hasMask) {
	    float alpha = texture(mainTex, IN.texCoord).a;
		
		if (alpha < 0.5) {
		    discard;
		}
	}
}