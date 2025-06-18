#version 330 core
out int FragId;

flat in int VertexId; // For mouse hovering, from the vertex shader in vertex attributes

void main()
{
	FragId = VertexId;
}