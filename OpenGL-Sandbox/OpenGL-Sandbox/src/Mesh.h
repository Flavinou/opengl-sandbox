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

    class BaseObject
    {
    public:
        void AddVertexBuffer(unsigned int& buffer);
        void SetVertexBufferData(const void* data, int size);
        void BindVertexBuffer(unsigned int buffer);
        void UnbindVertexBuffer();

        void BindVertexArray(unsigned int array);
        void UnbindVertexArray();

        void SetVertexAttribute(unsigned int index, unsigned int size, unsigned int type, bool normalized, unsigned int stride, const void* pointer) const;
        void SetVertexAttributeInstanceRate(unsigned int index, unsigned int rate);
    };

	class Mesh
	{
	public:
		// Constructor
		Mesh(const float* vertices, int verticesCount, int stride);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<MeshTexture>& textures);
		~Mesh();

		const unsigned int GetVAO() const { return m_VAO; }

		// Function to draw the mesh
		void Draw(const Shader& shader) const;
		void DrawInstanced(const Shader& shader, int instanceCount) const;
	private:
		void SetupMesh(); // Function to set up the mesh's OpenGL buffers and attributes
	private:
		unsigned int m_VAO, m_VBO, m_EBO; // Vertex Array Object, Vertex Buffer Object, Element Buffer Object IDs

		// Mesh data
		std::vector<Vertex> m_Vertices;			// List of vertices in the mesh
		std::vector<unsigned int> m_Indices;	// List of indices for indexed drawing
		std::vector<MeshTexture> m_Textures;	// List of textures applied to the mesh
	};

    class SimpleMesh : public BaseObject
    {
    public:
        SimpleMesh(const float* vertices, int size, int count); // Default constructor for empty mesh
        ~SimpleMesh();

        void Draw() const;
		void DrawInstanced(int instanceCount) const;
    private:
        unsigned int m_VAO, m_VBO; // Vertex Array Object and Vertex Buffer Object IDs

        std::vector<float> m_Vertices;
        int m_Count; // Number of vertices, size in bytes
    };
}