#version 330 core

in GS_OUT
{
	vec2 tCoords;
} fs_in;

uniform float fps;
uniform sampler2D depthTexture;
uniform sampler2D colorTexture;

uniform float width;
uniform float height;

uniform mat4 inv_MVP;
uniform mat4 prev_MVP;

uniform int smoke;
uniform int env;

uniform float pod_speed;

out vec4 color;

float linearizeDepth(float depth)
{
	float z = (depth * 2.0) - 1.0;
	return (2.0 * 1000.0) / (1001.0 - z * 999.0);
}

void main()
{
	float depth = linearizeDepth(texture(depthTexture, fs_in.tCoords).r);
	depth = depth / 1000.0;

	// compute view ray
	float xOffset = (width / 2.0);
	xOffset *= (fs_in.tCoords.x * 2.0 - 1.0);
	float yOffset = (height / 2.0);
	yOffset *= ((1.0 - fs_in.tCoords.y) * 2.0 - 1.0);

	vec3 horizontal_offset = vec3(1.0, 0.0, 0.0) * xOffset;
	vec3 vertical_offset = vec3(0.0, 1.0, 0.0) * yOffset;

	vec3 view_ray = vec3(0.0, 0.0, 1000.1) + horizontal_offset + vertical_offset;

	// compute world pos
	vec3 current = view_ray * depth;
	current = vec3(inv_MVP * vec4(current, 1.0));

	// compute previous screen space position
	vec4 previous = prev_MVP * vec4(current, 1.0);
	previous.xyz /= previous.w;
	previous.xy = previous.xy * 0.5 + 0.5;

	// compute blur vector
	vec2 blur_vector = previous.xy - fs_in.tCoords;

	// process blur
	float blurScale = fps / 60.0;
	if(env == 1)
		blur_vector *= 0.25 * (pod_speed / 300.0);
	else if(smoke == 1)
		blur_vector *= 0.1;

	color = texture(colorTexture, fs_in.tCoords);
	int samples = 15;
	for(int i = 1; i < samples; i++)
	{
		vec2 offset;
		if(env == 1)
			offset = blur_vector * (float(i) / float(samples - 1) - 0.5);
		else if(smoke == 1)
			offset = blur_vector * (float(i) / float(samples - 1));
		color += texture(colorTexture, fs_in.tCoords + offset);
	}
	color /= samples;
}
