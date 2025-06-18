#version 330 core
layout (location = 0) in vec3 aPos; // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal
layout (location = 2) in vec2 aTexCoords; // Vertex texture coordinates
layout (location = 3) in int aId; // Vertex ID (optional, can be used for instancing or other purposes)

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

flat out int VertexId; // Flat qualifier to ensure the ID is not interpolated

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
	FragPos = vec3(u_Model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(u_Model))) * aNormal;
	TexCoords = aTexCoords;
	VertexId = aId; // Pass the ID to the fragment shader

	gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}