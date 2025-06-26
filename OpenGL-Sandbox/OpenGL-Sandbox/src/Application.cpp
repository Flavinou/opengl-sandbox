#include "Application.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>

// Settings 
const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 900;

void SetLightingUniforms(const Application& app);

int main()
{
	auto* app = new Application(
        SCREEN_WIDTH, 
        SCREEN_HEIGHT, 
        Camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f)
    );

    app->Run();

	delete app;

    return 0;
}

void Application::FramebufferSizeCallback(int width, int height)
{
    glViewport(0, 0, width, height);
}

void Application::MouseCallback(double xPos, double yPos)
{
    if (m_FirstMouse)
    {
        m_LastMousePos = { (float)xPos, (float)yPos };
        m_FirstMouse = false;
    }

    glm::vec2 offset = 
    { 
        xPos - m_LastMousePos.x, 
        m_LastMousePos.y - yPos // reversed since y-coordinates range from bottom to top
    };
    m_LastMousePos = { (float)xPos, (float)yPos };

    m_Camera.OnMouseMove(offset);
}

void Application::ScrollCallback(double xOffset, double yOffset)
{
    m_Camera.OnMouseScroll(yOffset);
}

Application::Application(int viewportWidth, int viewportHeight, const Camera& camera)
	: m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight), m_Camera(camera)
{
	// GLFW initialization and window creation are handled in the application creation
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create the window
    m_Window = glfwCreateWindow(m_ViewportWidth, m_ViewportHeight, "OpenGL Sandbox", NULL, NULL);
    if (m_Window == NULL)
    {
        std::cout << "[ERROR]: Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_Window);

    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "[ERROR]: Failed to initialize GLAD!" << std::endl;
        return;
    }

    // Capture mouse by default when the application gets focus
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Allows us to access the Application instance from GLFW callbacks
    glfwSetWindowUserPointer(m_Window, this);

    // Set window callbacks
    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
    {
		auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		app->FramebufferSizeCallback(width, height);
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) 
    {
		auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		app->MouseCallback(xpos, ypos);
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset)
    {
		auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		app->ScrollCallback(xoffset, yoffset);
    });

    // Load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "[ERROR]: Failed to initialize GLAD!" << std::endl;
        return;
    }

	Initialize(); // Initialize OpenGL settings and resources
}

Application::~Application()
{
	// Cleanup OpenGL resources
	glDeleteFramebuffers(1, &m_Framebuffer);
	glDeleteRenderbuffers(1, &m_Renderbuffer);

    // Cleanup GLFW resources
    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
	}

    glfwTerminate();
}

void Application::Initialize()
{
	// Create light source mesh
    float cubeVertices[] =
    {   // positions            // normals              // texture coords
        -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,     1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,     1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,     0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,     1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     0.0f, 1.0f
    };

    // Plane vertices
    float planeVertices[] =
    {
        // positions            // normals          // texture coordinates (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,   2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,   0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,   2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,   0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,   2.0f, 2.0f
    };

	// Screen quad vertices (in normalized device coordinates)
    float quadVertices[] = {
        // positions   // texCoords
        //-1.0f,  1.0f,  0.0f, 1.0f,
        //-1.0f, -1.0f,  0.0f, 0.0f,
        // 1.0f, -1.0f,  1.0f, 0.0f,
        //
        //-1.0f,  1.0f,  0.0f, 1.0f,
        // 1.0f, -1.0f,  1.0f, 0.0f,
        // 1.0f,  1.0f,  1.0f, 1.0f
        
        // positions    // texCoords
        -0.25f,  1.0f,    0.0f, 1.0f,
        -0.25f,  0.75f,   0.0f, 0.0f,
         0.25f,  0.75f,   1.0f, 0.0f,

        -0.25f,  1.0f,    0.0f, 1.0f,
         0.25f,  0.75f,   1.0f, 0.0f,
         0.25f,  1.0f,    1.0f, 1.0f
    };

    m_PointLightPositions =
    {
        //glm::vec3(0.7f, 0.2f, 2.0f),
        //glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -2.5f),
        //glm::vec3(0.0f, 0.0f, -3.0f)
    };

    // Cubes positions
    m_CubePositions =
    {
        glm::vec3(-1.0f, 0.0f, -1.0f),
        glm::vec3(2.0f, 0.0f, 0.0f)
    };

    // Create shaders
    m_LitShader = std::make_shared<Shader>("resources/shaders/Vertex.glsl", "resources/shaders/LitFragment.glsl");
    m_UnlitShader = std::make_shared<Shader>("resources/shaders/Vertex.glsl", "resources/shaders/UnlitFragment.glsl");
    m_ScreenShader = std::make_shared<Shader>("resources/shaders/TextureVertex.glsl", "resources/shaders/TextureFragment.glsl");

	// Load textures
    m_GroundTexture = std::make_shared<Texture>("resources/textures/wall.jpg");
	m_CubeTexture = std::make_shared<Texture>("resources/textures/container2.png");

	// Create meshes
	m_PlaneMesh = std::make_shared<AssetLoader::Mesh>(planeVertices, sizeof(planeVertices) / sizeof(planeVertices[0]), 8);
	m_CubeMesh = std::make_shared<AssetLoader::Mesh>(cubeVertices, sizeof(cubeVertices) / sizeof(cubeVertices[0]), 8);
    m_LightSourceMesh = std::make_shared<AssetLoader::Mesh>(cubeVertices, sizeof(cubeVertices) / sizeof(cubeVertices[0]), 8);

    const int stride = 4 * sizeof(float);
    int size = sizeof(quadVertices);
    int count = size / stride;
    m_ScreenQuadMesh = std::make_shared<AssetLoader::SimpleMesh>(quadVertices, size, count);
	m_ScreenQuadMesh->SetVertexAttribute(0, 2, GL_FLOAT, false, stride, (void*)0); // Position attribute
	m_ScreenQuadMesh->SetVertexAttribute(1, 2, GL_FLOAT, false, stride, (void*)(2 * sizeof(float))); // Texture coordinate attribute

    // Bind the lit shader first to set the material shininess which is not meant to change
    m_LitShader->Use();
	m_LitShader->SetUniformFloat("u_Material.shininess", 32.0f);
	m_LitShader->SetUniformInt("u_Material.texture_diffuse1", 0); // Set the diffuse texture uniform to use texture unit 0

    m_ScreenShader->Use();
    m_ScreenShader->SetUniformInt("u_Texture", 0); // Set the texture uniform to use texture unit 0

    // Create framebuffer
    glGenFramebuffers(1, &m_Framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

    // Create a texture to render to
    m_ScreenTexture = std::make_shared<Texture>(m_ViewportWidth, m_ViewportHeight);

    // Attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ScreenTexture->GetID(), 0);

    // Create a renderbuffer object for depth and stencil testing
    glGenRenderbuffers(1, &m_Renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_ViewportWidth, m_ViewportHeight);

    // Attach the renderbuffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Renderbuffer);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "[ERROR]: Framebuffer is not complete!" << std::endl;
        return;
    }

    // Unbind the framebuffer to avoid accidental modifications
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::Run()
{
    // Main rendering loop
    while (!glfwWindowShouldClose(m_Window))
    {
        float currentFrame = (float)glfwGetTime();
        m_DeltaTime = currentFrame - m_LastFrameTime;
        m_LastFrameTime = currentFrame;

        // Handle user input
        ProcessInput();

        // Render the scene
        RenderScene();

		// Swap buffers and poll events
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
	}
}

void Application::ProcessInput()
{
    const float cameraSpeed = 2.5f * m_DeltaTime;

    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_Window, true);
    }

    // Camera movement
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
        m_Camera.OnKeyPressed(m_DeltaTime, CameraMovement::FORWARD);
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
        m_Camera.OnKeyPressed(m_DeltaTime, CameraMovement::BACKWARD);
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
        m_Camera.OnKeyPressed(m_DeltaTime, CameraMovement::LEFT);
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
        m_Camera.OnKeyPressed(m_DeltaTime, CameraMovement::RIGHT);
}

void Application::SetupLightUniforms()
{
    m_LitShader->Use();

    m_LitShader->SetVector3f("u_ViewPosition", m_Camera.GetWorldPosition());

    // Update the directional light uniforms
    m_LitShader->SetUniform3f("u_DirectionalLight.direction", -0.2f, -1.0f, -0.3f); // Directional light pointing downwards
    m_LitShader->SetUniform4f("u_DirectionalLight.ambient", 0.2f, 0.2f, 0.2f, 1.0f);
    m_LitShader->SetUniform4f("u_DirectionalLight.diffuse", 0.8f, 0.8f, 0.8f, 1.0f);
    m_LitShader->SetUniform4f("u_DirectionalLight.specular", 0.5f, 0.5f, 0.5f, 1.0f);

    // Update the point light uniforms
    const glm::vec3 pointLightAttenuationFactors{ 1.0f, 0.09f, 0.032f }; // Constant, linear and quadratic attenuation factors
    for (unsigned int i = 0; i < m_PointLightPositions.size(); i++)
    {
        std::string pointLightName;
        pointLightName.reserve(48); // Reserve space for the string to avoid reallocations
        pointLightName = "u_PointLights[" + std::to_string(i) + "]";
        m_LitShader->SetVector3f(pointLightName + ".position", { glm::sin(m_LastFrameTime) * m_PointLightPositions[i].x, m_PointLightPositions[i].y, glm::cos(m_LastFrameTime) * m_PointLightPositions[i].z });
        m_LitShader->SetUniform4f(pointLightName + ".ambient", 0.05f, 0.05f, 0.05f, 1.0f); // Ambient light color
        m_LitShader->SetUniform4f(pointLightName + ".diffuse", 0.8f, 0.8f, 0.8f, 1.0f); // Diffuse light color
        m_LitShader->SetUniform4f(pointLightName + ".specular", 1.0f, 1.0f, 1.0f, 1.0f); // Specular light color

        // We want the point light to cover a distance of 50 units, so we set the attenuation factors accordingly
        m_LitShader->SetUniformFloat(pointLightName + ".constant", pointLightAttenuationFactors.x);
        m_LitShader->SetUniformFloat(pointLightName + ".linear", pointLightAttenuationFactors.y);
        m_LitShader->SetUniformFloat(pointLightName + ".quadratic", pointLightAttenuationFactors.z);
    }

    // Update the spot light uniforms
    // The spot light is the camera itself, so we set its position to the camera's world position
    m_LitShader->SetVector3f("u_SpotLight.position", m_Camera.GetWorldPosition()); // Position of the spot light
    m_LitShader->SetVector3f("u_SpotLight.direction", m_Camera.GetForwardDirection()); // Direction of the spot light
    m_LitShader->SetUniform4f("u_SpotLight.ambient", 0.0f, 0.0f, 0.0f, 1.0f); // Ambient light color
    m_LitShader->SetUniform4f("u_SpotLight.diffuse", 1.0f, 1.0f, 1.0f, 1.0f); // Diffuse light color
    m_LitShader->SetUniform4f("u_SpotLight.specular", 1.0f, 1.0f, 1.0f, 1.0f); // Specular light color
    m_LitShader->SetUniformFloat("u_SpotLight.constant", pointLightAttenuationFactors.x);
    m_LitShader->SetUniformFloat("u_SpotLight.linear", pointLightAttenuationFactors.y);
    m_LitShader->SetUniformFloat("u_SpotLight.quadratic", pointLightAttenuationFactors.z);
    m_LitShader->SetUniformFloat("u_SpotLight.cutOff", glm::cos(glm::radians(5.0f))); // Inner cut-off angle for the spot light
    m_LitShader->SetUniformFloat("u_SpotLight.outerCutOff", glm::cos(glm::radians(17.5f))); // Outer cut-off angle for the spot light
}

void Application::RenderScene()
{
	// First pass, we render the scene with the camera rotated 180° along the y-axis to the framebuffer
    // The whole scene will be rendered "mirrored" to a texture that's 1/10th of the whole screen aspect ratio

	glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
	glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D rendering

    // Clear the framebuffer contents
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    SetupLightUniforms();

    m_LitShader->Use();

    // Set the model, view and projection matrix uniforms
    glm::mat4 projection = glm::perspective(glm::radians(m_Camera.GetFOV()), (float)m_ViewportWidth / (float)m_ViewportHeight, 0.1f, 100.0f);
    m_Camera.SetYaw(m_Camera.GetYaw() + 180.0f); // Rotate the camera 180 degrees around
    glm::mat4 view = m_Camera.GetViewMatrix();
    m_Camera.SetYaw(m_Camera.GetYaw() - 180.0f); // Reset camera orientation for next pass
    glm::mat4 model = glm::mat4(1.0f);

    m_LitShader->SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
    m_LitShader->SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader
    m_LitShader->SetMatrix4f("u_Model", model); // Set the model matrix for the shader

	// Render the plane
    m_GroundTexture->Bind();
    glm::mat4 planeModel = glm::mat4(1.0f);
    m_LitShader->SetMatrix4f("u_Model", planeModel);

	m_PlaneMesh->Draw(*m_LitShader);

    // Render the cubes
    m_CubeTexture->Bind();
    for (auto cubePosition : m_CubePositions)
    {
        // Calculate the model matrix for each cube
        glm::mat4 cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, cubePosition);
        m_LitShader->SetMatrix4f("u_Model", cubeModel);

        // Render cube
		m_CubeMesh->Draw(*m_LitShader);
    }

	// Render the point light sources
    m_UnlitShader->Use();

    glm::mat4 lightProjection = glm::perspective(glm::radians(m_Camera.GetFOV()), (float)m_ViewportWidth / (float)m_ViewportHeight, 0.1f, 100.0f);
    glm::mat4 lightView = m_Camera.GetViewMatrix();
    m_UnlitShader->SetMatrix4f("u_Projection", lightProjection); // Send the projection matrix to the shader
    m_UnlitShader->SetMatrix4f("u_View", lightView); // Pass the camera view matrix to the shader

    // Calculate the point lights model matrices and render them
    for (auto & PointLightPosition : m_PointLightPositions)
    {
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, { glm::sin(m_LastFrameTime) * PointLightPosition.x, PointLightPosition.y, glm::cos(m_LastFrameTime) * PointLightPosition.z });
        lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // Scale down the light source

        m_UnlitShader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
        m_UnlitShader->SetMatrix4f("u_Model", lightModel);

        // Render the light source model
        m_LightSourceMesh->Draw(*m_UnlitShader);
    }

    // Second pass, we render the scene normally to the default framebuffer

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D rendering

    // Clear the framebuffer contents
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    SetupLightUniforms();

    m_LitShader->Use();

    // Set the model, view and projection matrix uniforms
    projection = glm::perspective(glm::radians(m_Camera.GetFOV()), (float)m_ViewportWidth / (float)m_ViewportHeight, 0.1f, 100.0f);
    view = m_Camera.GetViewMatrix();
    model = glm::mat4(1.0f);

    m_LitShader->SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
    m_LitShader->SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader
    m_LitShader->SetMatrix4f("u_Model", model); // Set the model matrix for the shader

    // Render the plane
    m_GroundTexture->Bind();
    planeModel = glm::mat4(1.0f);
    m_LitShader->SetMatrix4f("u_Model", planeModel);

    m_PlaneMesh->Draw(*m_LitShader);

    // Render the cubes
    m_CubeTexture->Bind();
    for (auto cubePosition : m_CubePositions)
    {
        // Calculate the model matrix for each cube
        glm::mat4 cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, cubePosition);
        m_LitShader->SetMatrix4f("u_Model", cubeModel);

        // Render cube
        m_CubeMesh->Draw(*m_LitShader);
    }

    // Render the point light sources
    m_UnlitShader->Use();

    lightProjection = glm::perspective(glm::radians(m_Camera.GetFOV()), (float)m_ViewportWidth / (float)m_ViewportHeight, 0.1f, 100.0f);
    lightView = m_Camera.GetViewMatrix();
    m_UnlitShader->SetMatrix4f("u_Projection", lightProjection); // Send the projection matrix to the shader
    m_UnlitShader->SetMatrix4f("u_View", lightView); // Pass the camera view matrix to the shader

    // Calculate the point lights model matrices and render them
    for (auto& PointLightPosition : m_PointLightPositions)
    {
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, { glm::sin(m_LastFrameTime) * PointLightPosition.x, PointLightPosition.y, glm::cos(m_LastFrameTime) * PointLightPosition.z });
        lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // Scale down the light source

        m_UnlitShader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
        m_UnlitShader->SetMatrix4f("u_Model", lightModel);

        // Render the light source model
        m_LightSourceMesh->Draw(*m_UnlitShader);
    }

	// Last pass, we combine the main scene with the framebuffer texture at the top

    //glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the default framebuffer
    glDisable(GL_DEPTH_TEST); // Disable depth testing for the screen quad in order to render it on top of everything else
    
    // Clear the default framebuffer contents
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
    
    m_ScreenShader->Use();
    m_ScreenTexture->Bind(); // Bind the screen texture
    
    // Render a full-screen quad
    m_ScreenQuadMesh->Draw(); // Reuse the plane mesh to draw the full-screen quad
}