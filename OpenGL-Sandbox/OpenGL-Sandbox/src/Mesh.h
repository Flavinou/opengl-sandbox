#pragma once

#include "Shader.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace AssetLoader
{
	struct Vertex
	{
		glm::vec3 Position;  // Vertex position in 3D space
		glm::vec3 Normal;    // Vertex normal for lighting calculations
		glm::vec2 TexCoords; // Texture coordinates for mapping textures onto the vertex
	};

	struct Texture
	{
		unsigned int Id;	// Texture ID
		std::string Type;	// Type of texture (e.g., "diffuse", "specular")
	};

	class Mesh
	{
	public:
		// Constructor
		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures);

		// Function to draw the mesh
		void Draw(const Shader& shader);
	private:
		void SetupMesh(); // Function to set up the mesh's OpenGL buffers and attributes
	private:
		unsigned int m_VAO, m_VBO, m_EBO; // Vertex Array Object, Vertex Buffer Object, Element Buffer Object IDs

		// Mesh data
		std::vector<Vertex> m_Vertices;       // List of vertices in the mesh
		std::vector<unsigned int> m_Indices;  // List of indices for indexed drawing
		std::vector<Texture> m_Textures;      // List of textures applied to the mesh
	};
}