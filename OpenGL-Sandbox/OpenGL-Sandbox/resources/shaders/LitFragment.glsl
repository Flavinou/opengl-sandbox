#version 330 core
layout (location = 0) out vec4 FragColor;

struct Material 
{
	sampler2D texture_diffuse1;
	//	sampler2D texture_specular1;

	//	Test with max possible number of textures
	//	sampler2D textures[32];

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

#define MAX_POINT_LIGHTS 1

uniform vec3 u_ViewPosition;

uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight u_SpotLight;
uniform Material u_Material;

// Function prototypes
float LinearizeDepth(float depth);
vec4 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float near = 0.1;
float far = 100.0;

void main()
{
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(u_ViewPosition - FragPos);

	// Calculate lighting from directional light
	vec4 result = CalcDirLight(u_DirectionalLight, normal, viewDir);

	// Calculate lighting from point lights
	for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		result += CalcPointLight(u_PointLights[i], normal, FragPos, viewDir);
	}

	// Calculate lighting from spot light
	result += CalcSpotLight(u_SpotLight, normal, FragPos, viewDir);

	// Set the final fragment color
	FragColor = result;

	// Debug depth buffer
	// FragColor = vec4(vec3(LinearizeDepth(gl_FragCoord.z) / far), 1.0);

	// Debug texture
	// FragColor = texture(u_Material.texture_diffuse1, TexCoords);

	// Debug normals
	// FragColor = vec4(Normal.xyz, 1.0);
}

float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // back to normalized device coordinates
	return (2.0 * near * far) / (far + near - z * (far - near));
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
	vec4 ambient = light.ambient * texture(u_Material.texture_diffuse1, TexCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.texture_diffuse1, TexCoords);
	vec4 specular = light.specular * spec * texture(u_Material.texture_diffuse1, TexCoords);

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
	vec4 ambient = light.ambient * texture(u_Material.texture_diffuse1, TexCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.texture_diffuse1, TexCoords);
	vec4 specular = light.specular * spec * texture(u_Material.texture_diffuse1, TexCoords);
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
	vec4 ambient = light.ambient * texture(u_Material.texture_diffuse1, TexCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.texture_diffuse1, TexCoords);
	vec4 specular = light.specular * spec * texture(u_Material.texture_diffuse1, TexCoords);
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	return (ambient + diffuse + specular);
}