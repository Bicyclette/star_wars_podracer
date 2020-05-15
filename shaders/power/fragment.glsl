#version 330 core

uniform sampler2D img;

in GS_OUT
{
	vec2 texCoords;
} fs_in;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 bright_color;

uniform int depth;

void main()
{
	if(depth == 0)
	{
		color = texture(img, fs_in.texCoords);

		// extract bright color
		bright_color = color;
	}
}

