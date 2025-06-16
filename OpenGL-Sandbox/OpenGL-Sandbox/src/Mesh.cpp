#include "Mesh.h"

#include <glad/glad.h>

namespace AssetLoader
{
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<MeshTexture>& textures)
		: m_Vertices(vertices), m_Indices(indices), m_Textures(textures)
	{
		SetupMesh();
	}

    void Mesh::Draw(const Shader& shader) const
	{
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		for (unsigned int i = 0; i < m_Textures.size(); i++)
		{
			//glActiveTexture(GL_TEXTURE0 + i); // Activate proper texture unit before binding
			// Retrieve texture number (the "N" in diffuse_textureN)
			std::string number;
			std::string name = std::string(m_Textures[i].Type);
			if (name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = std::to_string(specularNr++);
			else
				continue; // Skip other types

			shader.SetUniformInt("u_Material." + name + number, i);
			m_Textures[i].Texture->Bind(i);
			//glBindTexture(GL_TEXTURE_2D, m_Textures[i].GetID());
		}
		glActiveTexture(GL_TEXTURE0); // Reset to default texture unit

		// Draw mesh
		glBindVertexArray(m_VAO);

		// Draw elements using indices - ONE draw call per mesh
		// There is room for optimization here, as we could batch draw calls if multiple meshes share the same textures
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_Indices.size()), GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0); // Unbind VAO
	}

	void Mesh::SetupMesh()
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);
	
		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		// Vertex Normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		// Vertex Texture Coordinates
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		glBindVertexArray(0); // Unbind VAO
	}
}

