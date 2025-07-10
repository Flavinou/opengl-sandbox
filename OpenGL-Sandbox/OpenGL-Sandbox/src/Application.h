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
	void SetLightingUniforms(const Shader& shader);
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
	std::shared_ptr<Shader> m_LitShader;
	std::shared_ptr<Shader> m_UnlitShader;
    std::shared_ptr<Shader> m_ShadowMapShader;
    std::shared_ptr<Shader> m_ScreenShader;

	std::shared_ptr<Texture> m_FloorTexture;

	std::shared_ptr<AssetLoader::Model> m_BackpackModel;

	std::shared_ptr<AssetLoader::Mesh> m_PlaneMesh;
	std::shared_ptr<AssetLoader::Mesh> m_LightSourceMesh;

    std::shared_ptr<AssetLoader::SimpleMesh> m_ScreenQuadMesh;

	// Utilities
	std::vector<glm::vec3> m_PointLightPositions;

	glm::vec3 m_LightPosition;

	// Shadow / depth mapping
	unsigned int m_DepthMapFramebuffer;
	std::shared_ptr<Texture> m_DepthMapTexture;
};