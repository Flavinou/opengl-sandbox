#include "Model.h"
#include "TextureManager.h"

#include <stb_image/stb_image.h>

#include <iostream>

namespace AssetLoader
{
	Model::Model(const std::string& path)
	{
		LoadModel(path);
	}

    Model::~Model()
    {
        /*for (const auto& mesh : m_Meshes)
        {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
            glDeleteBuffers(1, &m_EBO);
        }*/
    }

    void Model::Draw(const Shader& shader) const
	{
		for (const auto& mesh : m_Meshes)
		{
			mesh.Draw(shader);
		}
	}

	void Model::LoadModel(const std::string& path)
	{
		// Load the model using Assimp
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}

		m_Directory = path.substr(0, path.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// Process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(mesh, scene));
		}

		// Recursively process all the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		vertices.reserve(mesh->mNumVertices);

		std::vector<unsigned int> indices;
		std::vector<MeshTexture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex{};
			glm::vec3 vector{};

			// Positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			// Normals
			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}

			// Texture Coordinates
			if (mesh->HasTextureCoords(0)) // Does the mesh contain texture coordinates?
			{
				glm::vec2 vec{};
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
			{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}

			vertices.push_back(vertex);
		}

		// Indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// Materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			std::vector<MeshTexture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			std::vector<MeshTexture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		return { vertices, indices, textures };
	}

	std::vector<MeshTexture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
	{
		int textureCount = mat->GetTextureCount(type);
		std::vector<MeshTexture> textures;
		textures.reserve(textureCount);

		for (int i = 0; i < textureCount; i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);

			// If the texture is not loaded, load it
			// Otherwise it will get it from TextureManager memory
			const std::string texturePath = m_Directory + '/' + std::string(str.C_Str());
			auto texture = TextureManager::Instance().Get(texturePath);
			textures.push_back({ texture, typeName });
		}

		return textures;
	}
}

