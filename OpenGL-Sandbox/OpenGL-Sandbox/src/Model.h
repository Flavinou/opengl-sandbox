#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

namespace AssetLoader
{
	class Model
	{
	public:
        Model(const std::string& path);
		~Model();

		void Draw(const Shader& shader) const;
	private:
		void LoadModel(const std::string& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<MeshTexture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
	private:
		std::vector<Mesh> m_Meshes;
		std::string m_Directory;
	};
}