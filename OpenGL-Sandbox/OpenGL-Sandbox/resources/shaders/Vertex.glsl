#version 330 core
layout (location = 0) in vec3 aPos; // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal
layout (location = 2) in vec2 aTexCoords; // Vertex texture coordinates

out vec3 FragmentPosition;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
	FragmentPosition = vec3(u_Model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(u_Model))) * aNormal;
	TexCoords = aTexCoords;

	gl_Position = u_Projection * u_View * vec4(FragmentPosition, 1.0);
}