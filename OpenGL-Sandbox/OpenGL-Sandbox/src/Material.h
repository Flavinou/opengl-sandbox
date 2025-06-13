#pragma once

#include <glm/glm.hpp>

struct Material
{
	glm::vec4 Ambient;
	glm::vec4 Diffuse;
	glm::vec4 Specular;

	float Shininess;

	Material(const glm::vec4& ambient,
			 const glm::vec4& diffuse,
			 const glm::vec4& specular,
			 float shininess)
		: Ambient(ambient), Diffuse(diffuse), Specular(specular), Shininess(shininess) 
	{}
};

const Material Obsidian(glm::vec4(0.05375f, 0.05f, 0.06625f, 1.0f), // Ambient
						glm::vec4(0.18275f, 0.17f, 0.22525f, 1.0f), // Diffuse
						glm::vec4(0.332741f, 0.328634f, 0.346435f, 1.0f), // Specular
						38.4f); // Shininess

const Material Emerald(glm::vec4(0.0215f, 0.1745f, 0.0215f, 1.0f), // Ambient
						glm::vec4(0.07568f, 0.61424f, 0.07568f, 1.0f), // Diffuse
						glm::vec4(0.633f, 0.727811f, 0.633f, 1.0f), // Specular
						0.6f); // Shininess

const Material Ruby(glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f), // Ambient
					glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f), // Diffuse
					glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f), // Specular
					0.6f); // Shininess