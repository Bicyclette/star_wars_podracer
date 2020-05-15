#version 330 core

in GS_OUT
{
	vec2 tCoords;
} fs_in;

uniform sampler2D shadowMap;

out vec4 color;

float linearizeDepth(float depth)
{
	float z = (depth * 2.0) - 1.0;
	return (2.0 * 1000.0) / (1001.0 - z * (999.0));
}

void main()
{
	float depthValue = texture(shadowMap, fs_in.tCoords).r;
	float depth = linearizeDepth(depthValue);
	color = vec4(vec3(depth / 1000.0), 1.0);
}
