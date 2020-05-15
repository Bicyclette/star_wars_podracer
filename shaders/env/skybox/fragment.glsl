#version 330 core

out vec4 color;

in GS_OUT
{
	vec3 texCoords;
} fs_in;

uniform samplerCube skybox;

const float exposure = 2.0;

void main()
{
	color = texture(skybox, fs_in.texCoords);

	// exposure tone mapping
	color = vec4(vec3(1.0) - exp(-color.rgb * exposure), 1.0);
}

