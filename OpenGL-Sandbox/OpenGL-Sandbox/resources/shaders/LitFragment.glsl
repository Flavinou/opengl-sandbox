#version 330 core
out vec4 FragColor;

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

in VS_OUT
{
	vec3 FragPos; // Texture coordinates passed from the vertex shader
	vec3 Normal; // Normal vector in world space
	vec2 TexCoords; // Texture coordinates passed from the vertex shader
} fs_in; // Input from the vertex shader

#define MAX_POINT_LIGHTS 1

uniform vec3 u_ViewPosition;

uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight u_SpotLight;
uniform Material u_Material;

// Function prototypes
vec4 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec2 texCoords);
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);

void main()
{
	vec3 normal = normalize(fs_in.Normal);
	vec3 viewDir = normalize(u_ViewPosition - fs_in.FragPos);

	// Calculate lighting from directional light
	vec4 result = CalcDirLight(u_DirectionalLight, normal, viewDir, fs_in.TexCoords);

	// Calculate lighting from point lights
	for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		result += CalcPointLight(u_PointLights[i], normal, fs_in.FragPos, viewDir, fs_in.TexCoords);
	}

	// Calculate lighting from spot light
	result += CalcSpotLight(u_SpotLight, normal, fs_in.FragPos, viewDir, fs_in.TexCoords);

	// Set the final fragment color
	FragColor = result;

	// Debug texture
	// FragColor = texture(u_Material.texture_diffuse1, TexCoords);

	// Debug normals
	// FragColor = vec4(Normal.xyz, 1.0);
}

vec4 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec2 texCoords)
{
	vec3 lightDir = normalize(-light.direction);

	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

	// Combine results
	vec4 ambient = light.ambient * texture(u_Material.texture_diffuse1, texCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.texture_diffuse1, texCoords);
	vec4 specular = light.specular * spec * texture(u_Material.texture_diffuse1, texCoords);

	return (ambient + diffuse + specular);
}

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords)
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
	vec4 ambient = light.ambient * texture(u_Material.texture_diffuse1, texCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.texture_diffuse1, texCoords);
	vec4 specular = light.specular * spec * texture(u_Material.texture_diffuse1, texCoords);
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords)
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
	vec4 ambient = light.ambient * texture(u_Material.texture_diffuse1, texCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.texture_diffuse1, texCoords);
	vec4 specular = light.specular * spec * texture(u_Material.texture_diffuse1, texCoords);
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	return (ambient + diffuse + specular);
}