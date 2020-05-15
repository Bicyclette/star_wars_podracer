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

uniform vec3 center_ray;
uniform vec3 cam_up;
uniform vec3 cam_right;

uniform mat4 inv_modelView;
uniform mat4 prev_MVP;

uniform int smoke;
uniform int env;

uniform float pod_speed;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 bright_color;

float linearizeDepth(float depth)
{
	float z = (depth * 2.0) - 1.0;
	return (2.0 * 1000.0) / (1000.1 - z * 999.9);
}

void main()
{
    if(smoke == 1)
    {
    	float depth = linearizeDepth(texture(depthTexture, fs_in.tCoords).r);
    	depth = depth / 1000.0;

    	// compute view ray
    	float xOffset = (fs_in.tCoords.x * 2.0 - 1.0);
    	float yOffset = ((1.0 - fs_in.tCoords.y) * 2.0 - 1.0);

    	vec3 horizontal_offset = cam_right * -xOffset;
    	vec3 vertical_offset = cam_up * yOffset;

    	vec3 view_ray = center_ray + horizontal_offset + vertical_offset;

    	// compute world pos
    	vec3 current = view_ray * depth;
    	current = vec3(inv_modelView * vec4(current, 1.0));

    	// compute previous screen space position
    	vec4 previous = prev_MVP * vec4(current, 1.0);
    	previous.xyz /= previous.w;
    	previous.xy = previous.xy * 0.5 + 0.5;

    	// compute blur vector
    	vec2 blur_vector = previous.xy - fs_in.tCoords;

    	// process blur
    	float blurScale = fps / 60.0;
    	if(env == 1)
    		blur_vector *= (0.25 * (pod_speed / 300.0));
    	else if(smoke == 1)
    		blur_vector *= 0.045;

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
    else
    	color = texture(colorTexture, fs_in.tCoords);

	// extract bright color
	if(smoke == 1)
	{
		float brightness = dot(color.rgb * 5.5, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > 1.0)
			bright_color = color;
		else
			bright_color = vec4(0.0, 0.0, 0.0, 0.0);
	}
}
