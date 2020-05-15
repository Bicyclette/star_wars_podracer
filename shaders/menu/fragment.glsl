#version 330 core
uniform sampler2D img;

in GS_OUT
{
	vec2 tCoords;
} fs_in;

out vec4 frag_color;
uniform float alpha;
uniform int lap_timer_anim;
uniform float delta_anim;

float linearizeDepth(float depth)
{
	float z = (depth * 2.0) - 1.0;
	return (2.0 * 1000.0) / (1000.1 - z * 999.9);
}

void main()
{
	frag_color = texture(img, fs_in.tCoords);
	if(alpha != 1.0)
	{
		frag_color.a = alpha;
	}

    if(lap_timer_anim == 1)
    {
        frag_color = vec4(sin(delta_anim), 0.25, 0.15, frag_color.a);
    }
}

