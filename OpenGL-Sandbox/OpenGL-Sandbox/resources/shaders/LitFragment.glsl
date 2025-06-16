#version 330 core
out vec4 FragColor;

struct Material 
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct DirectionalLight 
{
	vec3 direction;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct PointLight
{
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;

	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

#define NB_POINT_LIGHTS 4

uniform vec3 u_ViewPosition;

uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLights[NB_POINT_LIGHTS];
uniform SpotLight u_SpotLight;
uniform Material u_Material;

// Function prototypes
vec4 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(u_ViewPosition - FragPos);

	// Calculate lighting from directional light
	vec4 result = CalcDirLight(u_DirectionalLight, normal, viewDir);

	// Calculate lighting from point lights
	for (int i = 0; i < NB_POINT_LIGHTS; ++i)
	{
		result += CalcPointLight(u_PointLights[i], normal, FragPos, viewDir);
	}

	// Calculate lighting from spot light
	result += CalcSpotLight(u_SpotLight, normal, FragPos, viewDir);

	// Set the final fragment color
	FragColor = result;
}

vec4 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

	// Combine results
	vec4 ambient = light.ambient * texture(u_Material.diffuse, TexCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.diffuse, TexCoords);
	vec4 specular = light.specular * spec * texture(u_Material.specular, TexCoords);

	return (ambient + diffuse + specular);
}

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);

	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);

	// Attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// Combine results
	vec4 ambient = light.ambient * texture(u_Material.diffuse, TexCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.diffuse, TexCoords);
	vec4 specular = light.specular * spec * texture(u_Material.specular, TexCoords);
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);

	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);

	// Attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// Spotlight effect
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	// Combine results
	vec4 ambient = light.ambient * texture(u_Material.diffuse, TexCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.diffuse, TexCoords);
	vec4 specular = light.specular * spec * texture(u_Material.specular, TexCoords);
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	return (ambient + diffuse + specular);
}