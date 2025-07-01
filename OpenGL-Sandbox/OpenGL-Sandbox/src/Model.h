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
	class Model : public BaseObject
	{
	public:
        Model(const std::string& path);
		~Model();

		const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const { return m_Meshes; }

		void Draw(const Shader& shader) const;
		void DrawInstanced(const Shader& shader, int instanceCount) const;
	private:
		void LoadModel(const std::string& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		std::shared_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<MeshTexture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
	private:
		std::vector<std::shared_ptr<Mesh>> m_Meshes;
		std::string m_Directory;
	};
}