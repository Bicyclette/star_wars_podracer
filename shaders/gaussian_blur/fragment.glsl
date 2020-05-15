#version 330 core

out vec4 color;

in GS_OUT
{
	vec2 tCoords;
} fs_in;

uniform sampler2D img;
uniform int horizontal;

void main()
{
	float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
	vec2 texOffset = 1.0 / textureSize(img, 0);
	vec3 result = texture(img, fs_in.tCoords).rgb * weights[0];
	if(horizontal == 1)
	{
		for(int i = 1; i < 5; i++)
		{
			result += texture(img, fs_in.tCoords + vec2(texOffset.x * i, 0.0)).rgb * weights[i];
			result += texture(img, fs_in.tCoords - vec2(texOffset.x * i, 0.0)).rgb * weights[i];
		}
	}
	else
	{
		for(int i = 1; i < 5; i++)
		{
			result += texture(img, fs_in.tCoords + vec2(texOffset.y * i, 0.0)).rgb * weights[i];
			result += texture(img, fs_in.tCoords - vec2(texOffset.y * i, 0.0)).rgb * weights[i];
		}
	}
	color = vec4(result, texture(img, fs_in.tCoords).a);
}
