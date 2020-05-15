#version 330 core

uniform sampler2D img;

in GS_OUT
{
	vec2 tCoords;
} fs_in;

out vec4 frag_color;

void main()
{
	frag_color = texture(img, fs_in.tCoords);
}

