#version 330 core

in GS_OUT
{
	float lifeTime;
	vec2 texCoords;
	flat int depth;
} fs_in;

out vec4 color;

uniform sampler2D f1;
uniform sampler2D f2;
uniform sampler2D f3;
uniform sampler2D f4;

void main()
{
	if(fs_in.depth == 0)
	{
		float deathTimer = 0.5;
		float ratio = fs_in.lifeTime / deathTimer;

		if(ratio < 0.0)
			ratio = 0.0;

		if(fs_in.lifeTime <= 0.025)
			color = texture(f2, fs_in.texCoords);
		else if(fs_in.lifeTime <= 0.2)
			color = texture(f2, fs_in.texCoords);
		else if(fs_in.lifeTime <= 0.375)
			color = texture(f2, fs_in.texCoords);
		else
			color = texture(f2, fs_in.texCoords);
        
        if(color.a == 0.0)
		{
			discard;
		}
		else
		{
			color = vec4(vec3(color), (1.0 + (1.0 - ratio) - ratio) * color.a * 0.7);
		}
	}
}

