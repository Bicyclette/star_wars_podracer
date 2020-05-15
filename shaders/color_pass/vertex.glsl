#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoords;

out VS_OUT
{
	vec2 tCoords;
} vs_out;

void main()
{
	vs_out.tCoords = texCoords;
	gl_Position = vec4(pos, 1.0);
}

