#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT
{
	vec3 Normal;
} gs_in[];

const float MAGNITUDE = 0.2;

uniform mat4 u_Projection;

void GenerateLine(int index);

void main()
{
	// Generate lines for each triangle edge
	GenerateLine(0);
	GenerateLine(1);
	GenerateLine(2);
}

void GenerateLine(int index)
{
	// Emit the start vertex
	gl_Position = u_Projection * gl_in[index].gl_Position;
	EmitVertex();

	// Emit the end vertex
	gl_Position = u_Projection * (gl_in[index].gl_Position + vec4(gs_in[index].Normal, 0.0) * MAGNITUDE);
	EmitVertex();
	EndPrimitive();
}