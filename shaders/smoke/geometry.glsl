#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices=5) out;

in VS_OUT
{
	float lifeTime;
	flat int depth;
} gs_in[];

out GS_OUT
{
	float lifeTime;
	vec2 texCoords;
	flat int depth;
} gs_out;

void main()
{
	float deathTimer = 0.5f;
	gs_out.lifeTime = gs_in[0].lifeTime;

	// compute percentage texture size increase
	float ratio = (gs_in[0].lifeTime / deathTimer);
	
	float max_width = 0.525;
	float max_height = 0.975;
	
	float min_width = 0.35;
	float min_height = 0.65;
	
	float width = max(min_width, ratio * max_width);
	float height = max(min_height, ratio * max_height);

	gl_Position = gl_in[0].gl_Position + vec4(-width, -height, 0.0f, 0.0f);
	gs_out.texCoords = vec2(0.0, 0.0);
	gs_out.depth = gs_in[0].depth;
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + vec4(width, -height, 0.0f, 0.0f);
	gs_out.texCoords = vec2(1.0, 0.0);
	gs_out.depth = gs_in[0].depth;
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + vec4(width, height, 0.0f, 0.0f);
	gs_out.texCoords = vec2(1.0, 1.0);
	gs_out.depth = gs_in[0].depth;
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + vec4(-width, height, 0.0f, 0.0f);
	gs_out.texCoords = vec2(0.0, 1.0);
	gs_out.depth = gs_in[0].depth;
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + vec4(-width, -height, 0.0f, 0.0f);
	gs_out.texCoords = vec2(0.0, 0.0);
	gs_out.depth = gs_in[0].depth;
	EmitVertex();
	
	EndPrimitive();
}

