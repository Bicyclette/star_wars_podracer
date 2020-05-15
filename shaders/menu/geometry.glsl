#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT
{
	vec2 tCoords;
} gs_in[];

out GS_OUT
{
	vec2 tCoords;
} gs_out;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	gs_out.tCoords = gs_in[0].tCoords;
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	gs_out.tCoords = gs_in[1].tCoords;
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	gs_out.tCoords = gs_in[2].tCoords;
	EmitVertex();

	EndPrimitive();
}

