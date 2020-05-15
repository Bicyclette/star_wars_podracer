#version 330 core

in GS_OUT
{
	vec2 tCoords;
} fs_in;

uniform sampler2D img;
out vec4 color;

void main()
{
	color = texture(img, fs_in.tCoords);
}
