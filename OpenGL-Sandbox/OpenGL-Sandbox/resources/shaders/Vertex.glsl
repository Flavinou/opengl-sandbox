#version 330 core
layout (location = 0) in vec3 aPos; // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal
layout (location = 2) in vec2 aTexCoords; // Vertex texture coordinates

out VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform bool u_ReverseNormals;

void main()
{
	vs_out.FragPos = vec3(u_Model * vec4(aPos, 1.0));
	if (u_ReverseNormals)
		vs_out.Normal = mat3(transpose(inverse(u_Model))) * (-1.0 * aNormal);
	else
		vs_out.Normal = mat3(transpose(inverse(u_Model))) * aNormal;
	vs_out.TexCoords = aTexCoords;

	gl_Position = u_Projection * u_View * vec4(vs_out.FragPos, 1.0);
}