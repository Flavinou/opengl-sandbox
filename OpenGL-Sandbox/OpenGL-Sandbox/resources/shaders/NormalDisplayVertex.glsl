#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT
{
	vec3 Normal; // Normal vector in world space
} vs_out;

uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_View * u_Model * vec4(aPos, 1.0);
	mat3 normalMatrix = mat3(transpose(inverse(u_View * u_Model)));
	vs_out.Normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
}