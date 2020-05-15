#version 330 core

in GS_OUT
{
	vec2 tCoords;
} fs_in;

uniform sampler2D img;
uniform int apply_greyScale;
const float offset_quit_game = 1.0 / 450.0;
const float offset_motion_blur = 1.0 / 300.0;
out vec4 color;

void main()
{
	float offset;
	if(apply_greyScale == 1)
		offset = offset_quit_game;
	else
		offset = offset_motion_blur;

	vec2 samples[9] = vec2[](
		vec2(-offset, offset),
		vec2(0.0, offset),
		vec2(offset, offset),
		vec2(-offset, 0.0),
		vec2(0.0, 0.0),
		vec2(offset, 0.0),
		vec2(-offset, -offset),
		vec2(0.0, -offset),
		vec2(offset, -offset)
	);

	float kernel[9] = float[](
		1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
		2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
		1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
	);

	vec3 sampleTex[9];
	for(int i = 0; i < 9; i++)
	{
		sampleTex[i] = vec3(texture(img, fs_in.tCoords.st + samples[i]));
	}

	vec3 blurColor = vec3(0.0);
	for(int i = 0; i < 9; i++)
	{
		blurColor += sampleTex[i] * kernel[i];
	}

	if(apply_greyScale == 1)
	{
		float greyScale = (0.2126 * blurColor.r) + (0.7152 * blurColor.g) + (0.0722 * blurColor.b);
		color = vec4(vec3(greyScale), 1.0);
	}
	else
	{
		color = vec4(blurColor, 1.0);
	}
}
