#version 430 core

uniform sampler2D 	mainTex;

in Vertex
{
	vec2 texCoord;
} IN;

void main(void)
{
	//float alpha = texture(mainTex, IN.texCoord).a;
	//if (alpha < 0.5) {
	//    discard;
	//}
}