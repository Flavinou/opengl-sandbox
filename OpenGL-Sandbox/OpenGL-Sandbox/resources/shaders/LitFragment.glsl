#version 330 core
out vec4 FragColor;

struct Material 
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light
{
	vec3 position;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

in vec3 FragmentPosition;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 u_ViewPosition;

uniform Material u_Material;
uniform Light u_Light;

void main()
{
	// Ambient
	vec4 ambient = u_Light.ambient * texture(u_Material.diffuse, TexCoords);

	// Diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDirection = normalize(u_Light.position - FragmentPosition);
	float diff = max(dot(norm, lightDirection), 0.0);
	vec4 diffuse = u_Light.diffuse * diff * texture(u_Material.diffuse, TexCoords);

	// Specular
	vec3 viewDirection = normalize(u_ViewPosition - FragmentPosition);
	vec3 reflectionDirection = reflect(-lightDirection, norm);
	float spec = pow(max(dot(viewDirection, reflectionDirection), 0.0), u_Material.shininess);
	vec4 specular = u_Light.specular * spec * texture(u_Material.specular, TexCoords);

	vec4 result = ambient + diffuse + specular;
	FragColor = result;
}