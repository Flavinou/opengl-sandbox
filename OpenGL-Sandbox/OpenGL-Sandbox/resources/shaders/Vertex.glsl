#version 330 core
layout (location = 0) in vec3 aPos; // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal

out vec3 FragmentPosition;
out vec3 Normal;
out vec3 LightViewPos; // Light position in view space
out vec4 LightingColor; // Vertex color for Gouraud shading

uniform vec3 u_LightPosition; // Light position in world space

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform vec4 u_LightColor;

void main()
{
	gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
	FragmentPosition = vec3(u_View * u_Model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(u_View * u_Model))) * aNormal;
	LightViewPos = vec3(u_View * vec4(u_LightPosition, 1.0)); // Transform light position to view space

	// Implement Gouraud shading
	float ambientStrength = 0.1;
	vec4 ambient = ambientStrength * u_LightColor;

	vec3 norm = normalize(Normal);
	vec3 lightDirection = normalize(LightViewPos - FragmentPosition);

	float diff = max(dot(norm, lightDirection), 0.0);
	vec4 diffuse = diff * u_LightColor;

	float specularStrength = 1.0;
	vec3 viewDirection = normalize(-FragmentPosition); // The viewer is always at the origin in view-space
	vec3 reflectionDirection = reflect(-lightDirection, norm);

	float spec = pow(max(dot(viewDirection, reflectionDirection), 0.0), 32);
	vec4 specular = specularStrength * spec * u_LightColor;

	LightingColor = ambient + diffuse + specular;
}