#version 330 core
uniform  sampler2D  diffuseTex;

in  Vertex{
	vec2  texCoord;
} IN;

out  vec4  fragColor;
void  main() {
	vec3 tex_color = texture(diffuseTex, IN.texCoord).rgb;

	fragColor = vec4(tex_color , 1.0);
}