#pragma once

#include "Shader.h"
#include "Texture.h"

#include <glm/glm.hpp>

#include <memory>
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

	struct MeshTexture
	{
		std::shared_ptr<Texture> Texture;
		std::string Type;
	};

	class Mesh
	{
	public:
		// Constructor
		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<MeshTexture>& textures);

		// Function to draw the mesh
		void Draw(const Shader& shader) const;
	private:
		void SetupMesh(); // Function to set up the mesh's OpenGL buffers and attributes
	private:
		unsigned int m_VAO, m_VBO, m_EBO; // Vertex Array Object, Vertex Buffer Object, Element Buffer Object IDs

		// Mesh data
		std::vector<Vertex> m_Vertices;			// List of vertices in the mesh
		std::vector<unsigned int> m_Indices;	// List of indices for indexed drawing
		std::vector<MeshTexture> m_Textures;	// List of textures applied to the mesh
	};
}