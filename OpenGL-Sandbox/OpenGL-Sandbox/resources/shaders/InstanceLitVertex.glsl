#version 330 core
layout (location = 0) in vec3 aPos; // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal
layout (location = 2) in vec2 aTexCoords; // Vertex texture coordinates
layout (location = 3) in mat4 aInstanceMatrix; // Instance model matrix

//out vec3 FragPos;
//out vec3 Normal;
//out vec2 TexCoords;

out VS_OUT 
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
	vs_out.FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
	vs_out.Normal = normalize(mat3(transpose(inverse(aInstanceMatrix))) * aNormal);
	vs_out.TexCoords = aTexCoords;

	gl_Position = u_Projection * u_View * vec4(vs_out.FragPos, 1.0);
}