#version 330 core
out vec4 FragColor;

struct Material 
{
	sampler2D texture_diffuse1;
	samplerCube shadow_map1;

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
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

#define MAX_POINT_LIGHTS 1

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;

uniform float u_FarPlane;

uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight u_SpotLight;
uniform Material u_Material;

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

// Function prototypes
vec4 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec2 texCoords);
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);

float CalcShadow(vec3 fragPos, vec3 normal, vec3 lightPos);

void main()
{
	vec3 normal = normalize(fs_in.Normal);
	vec3 viewDir = normalize(u_ViewPosition - fs_in.FragPos);

	// Calculate lighting from directional light
	vec4 result = vec4(0.0);
//	vec4 result = CalcDirLight(u_DirectionalLight, normal, viewDir, fs_in.TexCoords);

	// Calculate lighting from point lights
	for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		result += CalcPointLight(u_PointLights[i], normal, fs_in.FragPos, viewDir, fs_in.TexCoords);
	}

	// Calculate lighting from spot light
//	result += CalcSpotLight(u_SpotLight, normal, fs_in.FragPos, viewDir, fs_in.TexCoords);

	// Set the final fragment color
	FragColor = result;

	// Debug texture
	// FragColor = texture(u_Material.texture_diffuse1, TexCoords);

	// Debug normals
	// FragColor = vec4(Normal.xyz, 1.0);
}

vec4 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec2 texCoords)
{
	// For shadow casting
	vec3 lightDir = normalize(-light.direction);
//	vec3 lightDir = normalize(u_LightPosition - fs_in.FragPos);

	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular shading
	//vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);

	// Combine results
	vec4 ambient = light.ambient * texture(u_Material.texture_diffuse1, texCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.texture_diffuse1, texCoords);
	vec4 specular = light.specular * spec * texture(u_Material.texture_diffuse1, texCoords);
//	float shadow = CalcShadow(fs_in.FragPos, normal, lightDir, u_LightPosition);

//	return (ambient + (1.0 - shadow) * (diffuse + specular));
	return (ambient + diffuse + specular);
}

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords)
{
	vec3 lightDir = normalize(light.position - fragPos);

	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular shading
	//vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);

	// Attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// Combine results
	vec4 ambient = light.ambient * texture(u_Material.texture_diffuse1, texCoords);
	vec4 diffuse = light.diffuse * diff * texture(u_Material.texture_diffuse1, texCoords);
	vec4 specular = light.specular * spec * texture(u_Material.texture_diffuse1, texCoords);
	float shadow = CalcShadow(fs_in.FragPos, normal, u_LightPosition);
	//float shadow = CalcShadow(fs_in.FragPosLightSpace);
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	
//	return (ambient + diffuse + specular);
	return (ambient + (1.0 - shadow) * (diffuse + specular));
}

vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords)
{
	vec3 lightDir = normalize(light.position - fragPos);

	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular shading
	//vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);

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
	//float shadow = CalcShadow(fs_in.FragPosLightSpace);
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	
	return (ambient + diffuse + specular);
	//return (ambient + (1.0 - shadow) * (diffuse + specular));
}

float CalcShadow(vec3 fragPos, vec3 normal, vec3 lightPos)
{
	// get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(u_Material.shadow_map1, fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= u_FarPlane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
	float shadow = 0.0;
    float bias = 0.15;
	int samples = 20;
	float viewDistance = length(u_ViewPosition - fragPos);
	float diskRadius = (1.0 + (viewDistance / u_FarPlane)) / 25.0;
	for (int i = 0; i < samples; i++)
	{
		float closestDepth = texture(u_Material.shadow_map1, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= u_FarPlane; // undo mapping [0; 1]
		if (currentDepth - bias > closestDepth)
			shadow += 1;
	}

    shadow /= float(samples);

	// visualize depth as debug
	//	FragColor = vec4(vec3(closestDepth / u_FarPlane), 1.0); 

    return shadow;
}