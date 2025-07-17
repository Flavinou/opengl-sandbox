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
	vs_out.TexCoords = aTexCoords;

	vec3 normal = u_ReverseNormals ? -aNormal : aNormal;

	mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
	vs_out.Normal = normalize(normalMatrix * normal);

	gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}