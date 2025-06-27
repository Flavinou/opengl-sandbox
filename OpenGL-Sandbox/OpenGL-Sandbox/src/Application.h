#pragma once

#include "Camera.h"
#include "Model.h"
#include "Shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Application
{
public:
	Application(int viewportWidth, int viewportHeight, const Camera& camera);
	~Application();

	void Initialize();
	void Run();

	void ProcessInput();
	void RenderScene();
private:
	void SetupLightUniforms();
private:
	void FramebufferSizeCallback(int width, int height);
	void MouseCallback(double xpos, double ypos);
	void ScrollCallback(double xoffset, double yoffset);
private:
	int m_ViewportWidth, m_ViewportHeight;
	GLFWwindow* m_Window = nullptr;

	float m_LastFrameTime = 0.0f;
	float m_DeltaTime = 0.0f;

	// Input
	glm::vec2 m_LastMousePos{ 0.0f, 0.0f };
	bool m_FirstMouse = true;

	// Camera
	Camera m_Camera;

	// Resources
	unsigned int m_CubemapID = 0;

	std::shared_ptr<Cubemap> m_CubemapTexture;

	std::shared_ptr<Shader> m_LitShader;
	std::shared_ptr<Shader> m_UnlitShader;
	std::shared_ptr<Shader> m_CubemapShader;

	std::shared_ptr<AssetLoader::Model> m_BackpackModel;

	std::shared_ptr<AssetLoader::Mesh> m_LightSourceMesh;

	std::shared_ptr<AssetLoader::SimpleMesh> m_CubeMesh;
	std::shared_ptr<AssetLoader::SimpleMesh> m_CubemapMesh;

	// Utilities
	std::vector<glm::vec3> m_PointLightPositions;
	std::vector<std::string> m_TextureFaces;
};