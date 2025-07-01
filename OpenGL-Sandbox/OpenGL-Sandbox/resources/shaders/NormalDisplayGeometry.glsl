#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} gs_in[];

out GS_OUT
{
	vec3 FragPos; // Texture coordinates passed from the vertex shader
	vec3 Normal; // Normal vector in world space
	vec2 TexCoords; // Texture coordinates passed from the vertex shader
} gs_out; // Output to the fragment shader

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

	gs_out.TexCoords = gs_in[index].TexCoords; // Pass texture coordinates to the fragment shader
	gs_out.FragPos = vec3(gl_in[index].gl_Position); // Pass the position to the fragment shader
	gs_out.Normal = gs_in[index].Normal; // Pass the normal to the fragment shader
}