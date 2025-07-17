#version 330 core
out vec4 FragColor;

struct Material 
{
	sampler2D texture_diffuse1;

	float shininess;
};

struct DirectionalLight 
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in VS_OUT
{
	vec3 FragPos;
    vec3 Normal;
	vec2 TexCoords;
} fs_in;

#define MAX_POINT_LIGHTS 4

uniform vec3 u_ViewPosition;

uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform Material u_Material;

// Function prototypes
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec2 texCoords);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);

void main()
{
	vec3 normal = normalize(fs_in.Normal);
	vec3 viewDir = normalize(u_ViewPosition - fs_in.FragPos);

	// Calculate lighting from directional light
	vec4 result = vec4(0.0);
//	vec4 result = vec4(CalcDirLight(u_DirectionalLight, normal, viewDir, fs_in.TexCoords), 1.0);

	// Calculate lighting from point lights
	for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		result += vec4(CalcPointLight(u_PointLights[i], normal, fs_in.FragPos, viewDir, fs_in.TexCoords), 1.0);
	}

	// Set the final fragment color
	FragColor = result;

	// Debug texture
//	FragColor = texture(u_Material.texture_diffuse1, fs_in.TexCoords);

	// Debug normals
	// FragColor = vec4(fs_in.Normal.xyz, 1.0);
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec2 texCoords)
{
	vec3 lightDir = normalize(-light.direction);

	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);

	// Combine results
	vec3 ambient = light.ambient * texture(u_Material.texture_diffuse1, texCoords).rgb;
	vec3 diffuse = light.diffuse * diff * texture(u_Material.texture_diffuse1, texCoords).rgb;
	vec3 specular = light.specular * spec * texture(u_Material.texture_diffuse1, texCoords).rgb;

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords)
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
	vec3 ambient = light.ambient * texture(u_Material.texture_diffuse1, texCoords).rgb;
	vec3 diffuse = light.diffuse * diff * texture(u_Material.texture_diffuse1, texCoords).rgb;
	vec3 specular = light.specular * spec * texture(u_Material.texture_diffuse1, texCoords).rgb;
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}