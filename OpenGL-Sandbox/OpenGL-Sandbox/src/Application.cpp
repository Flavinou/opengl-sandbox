#include "Application.h"

#include "Core.h"
#include "TextureManager.h"

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

const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;

// Forward declaration
void RenderMesh(const AssetLoader::Mesh& mesh, const Shader& shader, const glm::mat4& transform);
void RenderSimpleMesh(const AssetLoader::SimpleMesh& mesh, const Shader& shader, const glm::mat4& transform);
std::shared_ptr<AssetLoader::SimpleMesh> CreateQuad();
std::shared_ptr<AssetLoader::SimpleMesh> CreateCube();

int main()
{
	Application* app = new Application(SCREEN_WIDTH, SCREEN_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f));

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
    glDeleteFramebuffers(1, &m_HDRFramebuffer);
    glDeleteRenderbuffers(1, &m_HDRDepthBuffer);

    // Cleanup GLFW resources
    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
	}

    glfwTerminate();
}

void Application::Initialize()
{
    GLCall(glEnable(GL_DEPTH_TEST));
    //GLCall(glEnable(GL_CULL_FACE));

    // Create shaders
    //m_UnlitShader = std::make_shared<Shader>("resources/shaders/Vertex.glsl", "resources/shaders/UnlitFragment.glsl");
    m_LitShader = std::make_shared<Shader>("resources/shaders/Vertex.glsl", "resources/shaders/LitFragment.glsl");
    m_HDRShader = std::make_shared<Shader>("resources/shaders/HDRVertex.glsl", "resources/shaders/HDRFragment.glsl");

    // Create model
    //m_BackpackModel = std::make_shared<AssetLoader::Model>("resources/models/backpack/backpack.obj");

	// Create light source mesh
    float cubeVertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-right
        // right face
         1.0f,  1.0f,  1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 1.0f, // top-right         
         1.0f, -1.0f, -1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 0.0f, // bottom-left     
         // bottom face
         -1.0f, -1.0f, -1.0f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // top-right
          1.0f, -1.0f, -1.0f,     0.0f, -1.0f,  0.0f,    1.0f, 1.0f, // top-left
          1.0f, -1.0f,  1.0f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // bottom-left
          1.0f, -1.0f,  1.0f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // bottom-left
         -1.0f, -1.0f,  1.0f,     0.0f, -1.0f,  0.0f,    0.0f, 0.0f, // bottom-right
         -1.0f, -1.0f, -1.0f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // top-right
         // top face
         -1.0f,  1.0f, -1.0f,     0.0f,  1.0f,  0.0f,    0.0f, 1.0f, // top-left
          1.0f,  1.0f , 1.0f,     0.0f,  1.0f,  0.0f,    1.0f, 0.0f, // bottom-right
          1.0f,  1.0f, -1.0f,     0.0f,  1.0f,  0.0f,    1.0f, 1.0f, // top-right     
          1.0f,  1.0f,  1.0f,     0.0f,  1.0f,  0.0f,    1.0f, 0.0f, // bottom-right
         -1.0f,  1.0f, -1.0f,     0.0f,  1.0f,  0.0f,    0.0f, 1.0f, // top-left
         -1.0f,  1.0f,  1.0f,     0.0f,  1.0f,  0.0f,    0.0f, 0.0f  // bottom-left        
    };

    // Plane vertices
    float planeVertices[] =
    {
        // positions            // normals          // texture coordinates (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         25.0f, -0.5f,  25.0f,    0.0f, 1.0f, 0.0f,   25.0f, 0.0f,
        -25.0f, -0.5f,  25.0f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
        -25.0f, -0.5f, -25.0f,    0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,    0.0f, 1.0f, 0.0f,   25.0f, 0.0f,
        -25.0f, -0.5f, -25.0f,    0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,    0.0f, 1.0f, 0.0f,   25.0f, 25.0f
    };

    // Light colors
    m_PointLightColors =
    {
        glm::vec3(200.0f, 200.0f, 200.0f),
        glm::vec3(0.1f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.2f),
        glm::vec3(0.0f, 0.1f, 0.0f)
    };

    m_PointLightPositions =
    {
        glm::vec3(0.0f,  0.0f, 49.5f), // back light
        glm::vec3(-1.4f, -1.9f, 9.0f),
        glm::vec3(0.0f, -1.8f, 4.0f),
        glm::vec3(0.8f, -1.7f, 6.0f)
    };

    // Create textures
    m_ToonWoodTexture = TextureManager::Instance().Get("resources/textures/toon_wood.jpg", true);

	// Create meshes
    m_QuadMesh = CreateQuad();
    m_CubeMesh = CreateCube();

    // Create HDR framebuffer
    GLCall(glGenFramebuffers(1, &m_HDRFramebuffer));

    // Create HDR color buffer
    m_HDRColorBuffer = std::make_shared<Texture>(m_ViewportWidth, m_ViewportHeight);
    m_HDRColorBuffer->SetData(nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    m_HDRColorBuffer->SetFilterMode(GL_LINEAR);

    // Create HDR depth buffer
    GLCall(glGenRenderbuffers(1, &m_HDRDepthBuffer));
    GLCall(glBindRenderbuffer(GL_RENDERBUFFER, m_HDRDepthBuffer));
    GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_ViewportWidth, m_ViewportHeight));

    // Attach the HDR color buffer and depth buffer to the HDR framebuffer
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFramebuffer));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_HDRColorBuffer->GetID(), 0));
    GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_HDRDepthBuffer));
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;

    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    // Bind the lit shader first to set the material shininess which is not meant to change
    m_LitShader->Use();
    m_LitShader->SetUniformFloat("u_Material.shininess", 32.0f);
    m_LitShader->SetUniformInt("u_Material.texture_diffuse1", 0);

    // Bind the HDR shader to setup texture slot
    m_HDRShader->Use();
    m_HDRShader->SetUniformInt("u_HDRTexture", 0);
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
        
        // Log HDR parameters
        std::cout << (m_UseHDR ? "HDR enabled." : "HDR disabled.") << " | Exposure : " << m_Exposure << std::endl;

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

    // HDR input
    if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_PRESS && !m_HDRKeyPressed)
    {
        m_UseHDR = !m_UseHDR;
        m_HDRKeyPressed = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        m_HDRKeyPressed = false;
    }

    if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (m_Exposure > 0.0f)
            m_Exposure -= 0.001f;
        else
            m_Exposure = 0.0f;
    }
    else if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS)
    {
        m_Exposure += 0.001f;
    }
}

void Application::RenderScene()
{
    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLCall(glViewport(0, 0, m_ViewportWidth, m_ViewportHeight));
    GLCall(glClearColor(0.15f, 0.15f, 0.15f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // First pass, render the (lit) scene to the HDR framebuffer
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFramebuffer));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    m_LitShader->Use();
    m_LitShader->SetVector3f("u_ViewPosition", m_Camera.GetWorldPosition());

    SetLightingUniforms(*m_LitShader);

    // Set the model, view and projection matrix uniforms
    glm::mat4 projection = glm::perspective(glm::radians(m_Camera.GetFOV()), (float)m_ViewportWidth / (float)m_ViewportHeight, 0.1f, 100.0f);
    glm::mat4 view = m_Camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    m_LitShader->SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
    m_LitShader->SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader

    // Inverse normals inside the tunnel
    m_LitShader->SetUniformBool("u_ReverseNormals", true);

    // First pass, draw the tunnel (lit scene)
    m_ToonWoodTexture->Bind();

    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 25.0f));
    model = glm::scale(model, glm::vec3(2.5f, 2.5f, 25.0f));
    RenderSimpleMesh(*m_CubeMesh, *m_LitShader, model);

    // Second pass, render HDR color buffer to 2D screen-filling quad with tone mapping shader
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    m_HDRShader->Use();
    m_HDRShader->SetUniformFloat("u_Exposure", m_Exposure);
    m_HDRShader->SetUniformBool("u_UseHDR", m_UseHDR);

    m_HDRColorBuffer->Bind();
    m_QuadMesh->Draw();
}

void Application::SetLightingUniforms(const Shader& shader)
{
    // Update the directional light uniforms
    //shader.SetUniform3f("u_DirectionalLight.direction", -0.2f, -1.0f, -0.3f); // Directional light pointing downwards
    //shader.SetUniform4f("u_DirectionalLight.ambient", 0.2f, 0.2f, 0.2f, 1.0f);
    //shader.SetUniform4f("u_DirectionalLight.diffuse", 0.8f, 0.8f, 0.8f, 1.0f);
    //shader.SetUniform4f("u_DirectionalLight.specular", 0.5f, 0.5f, 0.5f, 1.0f);

    // Update the point light uniforms
    const glm::vec3 pointLightAttenuationFactors{ 0.0f, 0.09f, 0.87f }; // Constant, linear and quadratic attenuation factors
    for (unsigned int i = 0; i < m_PointLightPositions.size(); i++)
    {
        std::string pointLightName;
        pointLightName.reserve(48); // Reserve space for the string to avoid reallocations
        pointLightName = "u_PointLights[" + std::to_string(i) + "]";
        shader.SetVector3f(pointLightName + ".position", m_PointLightPositions[i]);
        shader.SetVector3f(pointLightName + ".ambient", glm::vec3(0.0f)); // Ambient light color
        shader.SetVector3f(pointLightName + ".diffuse", m_PointLightColors[i]); // Diffuse light color
        shader.SetVector3f(pointLightName + ".specular", m_PointLightColors[i]); // Specular light color

        // We want the point light to cover a distance of ~50 units~ maximum a few units, so we set the attenuation factors accordingly
        shader.SetUniformFloat(pointLightName + ".constant", pointLightAttenuationFactors.x);
        shader.SetUniformFloat(pointLightName + ".linear", pointLightAttenuationFactors.y);
        shader.SetUniformFloat(pointLightName + ".quadratic", pointLightAttenuationFactors.z);
    }
}

void RenderMesh(const AssetLoader::Mesh& mesh, const Shader& shader, const glm::mat4& transform)
{
    shader.SetMatrix4f("u_Model", transform);
    mesh.Draw(shader);
}

void RenderSimpleMesh(const AssetLoader::SimpleMesh& mesh, const Shader& shader, const glm::mat4& transform)
{
    shader.SetMatrix4f("u_Model", transform);
    mesh.Draw();
}

std::shared_ptr<AssetLoader::SimpleMesh> CreateQuad()
{
    // positions
    glm::vec3 pos1(-1.0, 1.0, 0.0);
    glm::vec3 pos2(-1.0, -1.0, 0.0);
    glm::vec3 pos3(1.0, -1.0, 0.0);
    glm::vec3 pos4(1.0, 1.0, 0.0);
    // texture coordinates
    glm::vec2 uv1(0.0, 1.0);
    glm::vec2 uv2(0.0, 0.0);
    glm::vec2 uv3(1.0, 0.0);
    glm::vec2 uv4(1.0, 1.0);

    float quadVertices[] = {
        // positions            // texcoords 
        pos1.x, pos1.y, pos1.z, uv1.x, uv1.y,
        pos2.x, pos2.y, pos2.z, uv2.x, uv2.y,
        pos3.x, pos3.y, pos3.z, uv3.x, uv3.y,

        pos1.x, pos1.y, pos1.z, uv1.x, uv1.y,
        pos3.x, pos3.y, pos3.z, uv3.x, uv3.y,
        pos4.x, pos4.y, pos4.z, uv4.x, uv4.y
    };

    const int stride = 5 * sizeof(float);
    int size = sizeof(quadVertices);
    int count = size / stride;
    std::shared_ptr<AssetLoader::SimpleMesh> simpleMesh = std::make_shared<AssetLoader::SimpleMesh>(quadVertices, size, count);
    simpleMesh->SetVertexAttribute(0, 3, GL_FLOAT, false, stride, nullptr);
    simpleMesh->SetVertexAttribute(1, 2, GL_FLOAT, false, stride, (void*)(3 * sizeof(float)));

    return simpleMesh;
}

std::shared_ptr<AssetLoader::SimpleMesh> CreateCube()
{
    float cubeVertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
         // bottom face
         -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
          1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
          1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
          1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
         -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
         -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
         // top face
         -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
          1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
          1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
          1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
         -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
         -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
    };

    const int cubeStride = 8 * sizeof(float);
    int cubeSize = sizeof(cubeVertices);
    int cubeVerticesCount = cubeSize / cubeStride;
    std::shared_ptr<AssetLoader::SimpleMesh> cubeMesh = std::make_shared<AssetLoader::SimpleMesh>(cubeVertices, sizeof(cubeVertices), cubeVerticesCount);
    cubeMesh->SetVertexAttribute(0, 3, GL_FLOAT, false, cubeStride, nullptr);
    cubeMesh->SetVertexAttribute(1, 3, GL_FLOAT, false, cubeStride, (void*)(3 * sizeof(float)));
    cubeMesh->SetVertexAttribute(2, 2, GL_FLOAT, false, cubeStride, (void*)(6 * sizeof(float)));

    return cubeMesh;
}