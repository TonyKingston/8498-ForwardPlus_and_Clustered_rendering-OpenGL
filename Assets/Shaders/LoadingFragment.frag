#version 330 core
uniform  sampler2D  nyan;

in  Vertex{
	vec2  texCoord;
} IN;

out  vec4  fragColor;
void  main() {
	fragColor = texture(nyan, IN.texCoord*8.0f);
}