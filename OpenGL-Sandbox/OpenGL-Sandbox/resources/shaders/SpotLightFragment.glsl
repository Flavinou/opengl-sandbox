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
	vec3 position; // Spot light here
	vec3 direction;
	float cutoff; // Cutoff angle for spotlight
	float outerCutoff; // Outer cutoff angle for spotlight

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	// Attenuation parameters (used for point lights)
	float constant;
	float linear;
	float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 u_ViewPosition;
uniform Material u_Material;
uniform Light u_Light;

void main()
{
	vec3 lightDirection = normalize(u_Light.position - FragPos);

	// Ambient
	vec4 ambient = u_Light.ambient * texture(u_Material.diffuse, TexCoords);

	// Diffuse
	vec3 norm = normalize(Normal);
	float diff = max(dot(norm, lightDirection), 0.0);
	vec4 diffuse = u_Light.diffuse * diff * texture(u_Material.diffuse, TexCoords);

	// Specular
	vec3 viewDirection = normalize(u_ViewPosition - FragPos);
	vec3 reflectionDirection = reflect(-lightDirection, norm);
	float spec = pow(max(dot(viewDirection, reflectionDirection), 0.0), u_Material.shininess);
	vec4 specular = u_Light.specular * spec * texture(u_Material.specular, TexCoords);

	// Check if the light is within the spotlight's cutoff angle
	float theta = dot(lightDirection, normalize(-u_Light.direction)); // Assuming the light direction is pointing towards the light source
	float epsilon = u_Light.cutoff - u_Light.outerCutoff;
	float intensity = clamp((theta - u_Light.outerCutoff) / epsilon, 0.0, 1.0);
	diffuse *= intensity; // Apply intensity to diffuse light
	specular *= intensity; // Apply intensity to specular light

	// Apply attenuation to the light components
	float distance = length(u_Light.position - FragPos);
	float attenuation = 1.0 / (u_Light.constant + u_Light.linear * distance + u_Light.quadratic * (distance * distance));
	ambient *= attenuation; // Ambient light is not usually attenuated, at large distances the light would be darker inside than outside due to attenuation in the else branch
	diffuse *= attenuation;
	specular *= attenuation;

	vec4 result = ambient + diffuse + specular;
	FragColor = result;
}