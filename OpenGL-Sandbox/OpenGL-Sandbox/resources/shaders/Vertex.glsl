#version 330 core
layout (location = 0) in vec3 aPos; // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal
layout (location = 2) in vec2 aTexCoords; // Vertex texture coordinates
layout (location = 3) in vec3 aTangent; // Tangent vector
layout (location = 4) in vec3 aBitangent; // Vertex texture coordinates

out VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
	vec3 TangentLightPos;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
} vs_out;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;

void main()
{
	vs_out.FragPos = vec3(u_Model * vec4(aPos, 1.0));
	vs_out.TexCoords = aTexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));    
    vs_out.TangentLightPos = TBN * u_LightPosition;
    vs_out.TangentViewPos  = TBN * u_ViewPosition;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;

	gl_Position = u_Projection * u_View * vec4(vs_out.FragPos, 1.0);
}