#version 330 core
out vec4 FragColor;

in vec3 FragmentPosition;
in vec3 Normal;
in vec3 LightViewPos;

uniform vec4 u_LightColor;
uniform vec4 u_TintColor;

void main()
{
	float ambientStrength = 0.1;
	vec4 ambient = ambientStrength * u_LightColor;

	vec3 norm = normalize(Normal);
	vec3 lightDirection = normalize(LightViewPos - FragmentPosition);

	float diff = max(dot(norm, lightDirection), 0.0);
	vec4 diffuse = diff * u_LightColor;

	float specularStrength = 0.5;
	vec3 viewDirection = normalize(-FragmentPosition); // The viewer is always at the origin in view-space
	vec3 reflectionDirection = reflect(-lightDirection, norm);

	float spec = pow(max(dot(viewDirection, reflectionDirection), 0.0), 32);
	vec4 specular = specularStrength * spec * u_LightColor;

	vec4 result = (ambient + diffuse + specular) * u_TintColor;
	FragColor = result;
}