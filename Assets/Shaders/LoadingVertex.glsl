#version  330  core

uniform float movement;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

out  Vertex{
	vec2 texCoord;
} OUT;

void  main() {
	vec3 offset = vec3(movement, 0.3f, 0.0f);
	gl_Position = vec4(position + offset, 1.0);
	OUT.texCoord = texCoord;
}