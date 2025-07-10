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

    // Create shaders
    m_LitShader = std::make_shared<Shader>("resources/shaders/Vertex.glsl", "resources/shaders/LitFragment.glsl");
    m_UnlitShader = std::make_shared<Shader>("resources/shaders/Vertex.glsl", "resources/shaders/UnlitFragment.glsl");
    m_ShadowMapShader = std::make_shared<Shader>("resources/shaders/ShadowMappingVertex.glsl", "resources/shaders/ShadowMappingFragment.glsl");
    m_ScreenShader = std::make_shared<Shader>("resources/shaders/DebugQuadVertex.glsl", "resources/shaders/DebugQuadDepthFragment.glsl");

    // Create model
    m_BackpackModel = std::make_shared<AssetLoader::Model>("resources/models/backpack/backpack.obj");

	// Create light source mesh
    //float cubeVertices[] =
    //{   // positions            // normals              // texture coords
    //    -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     0.0f, 0.0f,
    //     0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f, 0.0f,
    //     0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
    //     0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
    //    -0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     0.0f, 1.0f,
    //    -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     0.0f, 0.0f,

    //    -0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      0.0f, 0.0f,
    //     0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      1.0f, 0.0f,
    //     0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      1.0f, 1.0f,
    //     0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      1.0f, 1.0f,
    //    -0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      0.0f, 1.0f,
    //    -0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      0.0f, 0.0f,

    //    -0.5f,  0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,     1.0f, 0.0f,
    //    -0.5f,  0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,     1.0f, 1.0f,
    //    -0.5f, -0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
    //    -0.5f, -0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
    //    -0.5f, -0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,     0.0f, 0.0f,
    //    -0.5f,  0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,     1.0f, 0.0f,

    //     0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f, 0.0f,
    //     0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     1.0f, 1.0f,
    //     0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
    //     0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
    //     0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     0.0f, 0.0f,
    //     0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f, 0.0f,

    //    -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     0.0f, 1.0f,
    //     0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     1.0f, 1.0f,
    //     0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f, 0.0f,
    //     0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f, 0.0f,
    //    -0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     0.0f, 0.0f,
    //    -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     0.0f, 1.0f,

    //    -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     0.0f, 1.0f,
    //     0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     1.0f, 1.0f,
    //     0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f, 0.0f,
    //     0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f, 0.0f,
    //    -0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     0.0f, 0.0f,
    //    -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     0.0f, 1.0f
    //};

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

    //float quadVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    //    // positions   // texCoords
    //    -1.0f,  1.0f,  0.0f, 1.0f,
    //    -1.0f, -1.0f,  0.0f, 0.0f,
    //     1.0f, -1.0f,  1.0f, 0.0f,

    //    -1.0f,  1.0f,  0.0f, 1.0f,
    //     1.0f, -1.0f,  1.0f, 0.0f,
    //     1.0f,  1.0f,  1.0f, 1.0f
    //};

    m_PointLightPositions =
    {
        //glm::vec3(0.7f, 0.2f, 2.0f),
        //glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -2.5f),
        //glm::vec3(0.0f, 0.0f, -3.0f)
    };

    // Create textures
    m_FloorTexture = TextureManager::Instance().Get("resources/textures/proto_wall_orange.png");

	// Create meshes
    m_PlaneMesh = std::make_shared<AssetLoader::Mesh>(planeVertices, sizeof(planeVertices) / sizeof(planeVertices[0]), 8);
    m_LightSourceMesh = std::make_shared<AssetLoader::Mesh>(cubeVertices, sizeof(cubeVertices) / sizeof(cubeVertices[0]), 8);

    //const int stride = 4 * sizeof(float);
    //int size = sizeof(quadVertices);
    //int count = size / stride;
    //m_ScreenQuadMesh = std::make_shared<AssetLoader::SimpleMesh>(quadVertices, size, count);
    //m_ScreenQuadMesh->SetVertexAttribute(0, 2, GL_FLOAT, false, stride, (void*)0); // Position attribute
    //m_ScreenQuadMesh->SetVertexAttribute(1, 2, GL_FLOAT, false, stride, (void*)(2 * sizeof(float))); // Texture coordinate attribute

    // Create shadow / depth map framebuffer
    GLCall(glGenFramebuffers(1, &m_DepthMapFramebuffer));

    // Create depth map texture
    m_DepthMapTexture = TextureManager::Instance().Get("ShadowMap", SHADOW_WIDTH, SHADOW_HEIGHT);
    m_DepthMapTexture->SetData(nullptr, GL_DEPTH_COMPONENT);
    m_DepthMapTexture->SetFilterMode(GL_NEAREST);
    m_DepthMapTexture->SetWrapMode(GL_CLAMP_TO_BORDER);
    m_DepthMapTexture->SetBorderColor(glm::vec4(1.0f));

    // Attach depth texture to shadow map framebuffer
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFramebuffer));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthMapTexture->GetID(), 0));
    GLCall(glDrawBuffer(GL_NONE));
    GLCall(glReadBuffer(GL_NONE));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    // Bind the lit shader first to set the material shininess which is not meant to change
    m_LitShader->Use();
    m_LitShader->SetUniformFloat("u_Material.shininess", 32.0f);
    m_LitShader->SetUniformInt("u_Material.shadow_map1", 1);

    //m_ScreenShader->Use();
    //m_ScreenShader->SetUniformInt("u_DepthMap", 0);

    m_LightPosition = glm::vec3(-2.0f, 4.0f, -1.0f);
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

void Application::RenderScene()
{
    // Rendering anything happens here
    GLCall(glClearColor(0.15f, 0.15f, 0.15f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // First pass, render to depth / shadow map (from light position perspective)
    const float nearPlane = 1.0f, farPlane = 7.5f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(m_LightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    glm::mat4 lightModel = glm::mat4(1.0f);

    m_ShadowMapShader->Use();
    m_ShadowMapShader->SetMatrix4f("u_LightSpace", lightSpaceMatrix);

    GLCall(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFramebuffer));
    GLCall(glClear(GL_DEPTH_BUFFER_BIT));

    // Draw the floor
    m_FloorTexture->Bind();

    //lightModel = glm::translate(lightModel, glm::vec3(0.0f, -1.5f, 0.0f));
    m_ShadowMapShader->SetMatrix4f("u_Model", lightModel);
    m_PlaneMesh->Draw(*m_ShadowMapShader);

    // Draw the backpack model
    //lightModel = glm::mat4(1.0f);
    //lightModel = glm::translate(lightModel, glm::vec3(0.0f, 0.0f, -6.0f));
    //m_ShadowMapShader->SetMatrix4f("u_Model", lightModel);
    //m_BackpackModel->Draw(*m_ShadowMapShader); // Draw the backpack model with the lit shader

    // Draw cubes (using bound wood texture)
    // "m_LightSourceMesh" is just a cube mesh scaled down to display the position of point lights
    m_FloorTexture->Bind();
    glm::mat4 cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, glm::vec3(0.0f, 1.5f, 0.0f));
    cubeModel = glm::scale(cubeModel, glm::vec3(0.5f));
    RenderMesh(*m_LightSourceMesh, *m_ShadowMapShader, cubeModel);

    cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, glm::vec3(2.0f, 0.0f, 1.0f));
    cubeModel = glm::scale(cubeModel, glm::vec3(0.5f));
    RenderMesh(*m_LightSourceMesh, *m_ShadowMapShader, cubeModel);

    cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, glm::vec3(-1.0f, 0.0f, 2.0f));
    cubeModel = glm::rotate(cubeModel, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    cubeModel = glm::scale(cubeModel, glm::vec3(0.25f));
    RenderMesh(*m_LightSourceMesh, *m_ShadowMapShader, cubeModel);

    // Calculate the point lights model matrices and render them
    for (unsigned int i = 0; i < m_PointLightPositions.size(); i++)
    {
        glm::mat4 pointLightModel = glm::mat4(1.0f);
        pointLightModel = glm::translate(pointLightModel, { glm::sin(m_LastFrameTime) * m_PointLightPositions[i].x, m_PointLightPositions[i].y, glm::cos(m_LastFrameTime) * m_PointLightPositions[i].z });
        pointLightModel = glm::scale(pointLightModel, glm::vec3(0.2f)); // Scale down the light source

        m_ShadowMapShader->SetMatrix4f("u_Model", pointLightModel);

        // Render the light source model
        m_LightSourceMesh->Draw(*m_ShadowMapShader);
    }

    // Second pass, render the scene normally using the generated depth/ shadow map
    // Reset viewport
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GLCall(glViewport(0, 0, m_ViewportWidth, m_ViewportHeight));
    GLCall(glClearColor(0.15f, 0.15f, 0.15f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    m_LitShader->Use();
    m_LitShader->SetVector3f("u_ViewPosition", m_Camera.GetWorldPosition());

    SetLightingUniforms(*m_LitShader);

    // Set the model, view and projection matrix uniforms
    glm::mat4 projection = glm::perspective(glm::radians(m_Camera.GetFOV()), (float)m_ViewportWidth / (float)m_ViewportHeight, 0.1f, 100.0f);
    glm::mat4 view = m_Camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    m_LitShader->SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
    m_LitShader->SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader

    //model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
    m_LitShader->SetMatrix4f("u_Model", model); // Set the model matrix for the shader

    // Shadow casting
    m_LitShader->SetVector3f("u_LightPosition", m_LightPosition);
    m_LitShader->SetMatrix4f("u_LightSpace", lightSpaceMatrix);

    // Draw the floor
    m_FloorTexture->Bind();
    m_DepthMapTexture->Bind(1);
    m_PlaneMesh->Draw(*m_LitShader);

    // Draw the backpack model
    //model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(0.0f, 0.0f, -6.0f));
    //m_LitShader->SetMatrix4f("u_Model", model);
    //m_BackpackModel->Draw(*m_LitShader); // Draw the backpack model with the lit shader

    // Draw cubes (using bound wood texture)
    // "m_LightSourceMesh" is just a cube mesh scaled down to display the position of point lights
    m_FloorTexture->Bind();
    cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, glm::vec3(0.0f, 1.5f, 0.0f));
    cubeModel = glm::scale(cubeModel, glm::vec3(0.5f));
    RenderMesh(*m_LightSourceMesh, *m_LitShader, cubeModel);

    cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, glm::vec3(2.0f, 0.0f, 1.0f));
    cubeModel = glm::scale(cubeModel, glm::vec3(0.5f));
    RenderMesh(*m_LightSourceMesh, *m_LitShader, cubeModel);

    cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, glm::vec3(-1.0f, 0.0f, 2.0f));
    cubeModel = glm::rotate(cubeModel, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    cubeModel = glm::scale(cubeModel, glm::vec3(0.25f));
    RenderMesh(*m_LightSourceMesh, *m_LitShader, cubeModel);

    m_UnlitShader->Use();
    m_UnlitShader->SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
    m_UnlitShader->SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader

    // Calculate the point lights model matrices and render them
    for (unsigned int i = 0; i < m_PointLightPositions.size(); i++)
    {
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, { glm::sin(m_LastFrameTime) * m_PointLightPositions[i].x, m_PointLightPositions[i].y, glm::cos(m_LastFrameTime) * m_PointLightPositions[i].z });
        lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // Scale down the light source

        m_UnlitShader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
        m_UnlitShader->SetMatrix4f("u_Model", lightModel);

        // Render the light source model
        m_LightSourceMesh->Draw(*m_UnlitShader);
    }
}

void Application::SetLightingUniforms(const Shader& shader)
{
    // Update the directional light uniforms
    shader.SetUniform3f("u_DirectionalLight.direction", -0.2f, -1.0f, -0.3f); // Directional light pointing downwards
    shader.SetUniform4f("u_DirectionalLight.ambient", 0.2f, 0.2f, 0.2f, 1.0f);
    shader.SetUniform4f("u_DirectionalLight.diffuse", 0.8f, 0.8f, 0.8f, 1.0f);
    shader.SetUniform4f("u_DirectionalLight.specular", 0.5f, 0.5f, 0.5f, 1.0f);

    // Update the point light uniforms
    const glm::vec3 pointLightAttenuationFactors{ 1.0f, 0.09f, 0.032f }; // Constant, linear and quadratic attenuation factors
    for (unsigned int i = 0; i < m_PointLightPositions.size(); i++)
    {
        std::string pointLightName;
        pointLightName.reserve(48); // Reserve space for the string to avoid reallocations
        pointLightName = "u_PointLights[" + std::to_string(i) + "]";
        shader.SetVector3f(pointLightName + ".position", { glm::sin(m_LastFrameTime) * m_PointLightPositions[i].x, m_PointLightPositions[i].y, glm::cos(m_LastFrameTime) * m_PointLightPositions[i].z });
        shader.SetUniform4f(pointLightName + ".ambient", 0.05f, 0.05f, 0.05f, 1.0f); // Ambient light color
        shader.SetUniform4f(pointLightName + ".diffuse", 0.8f, 0.8f, 0.8f, 1.0f); // Diffuse light color
        shader.SetUniform4f(pointLightName + ".specular", 1.0f, 1.0f, 1.0f, 1.0f); // Specular light color

        // We want the point light to cover a distance of 50 units, so we set the attenuation factors accordingly
        shader.SetUniformFloat(pointLightName + ".constant", pointLightAttenuationFactors.x);
        shader.SetUniformFloat(pointLightName + ".linear", pointLightAttenuationFactors.y);
        shader.SetUniformFloat(pointLightName + ".quadratic", pointLightAttenuationFactors.z);
    }

    // Update the spot light uniforms
    // The spot light is the camera itself, so we set its position to the camera's world position
    shader.SetVector3f("u_SpotLight.position", m_Camera.GetWorldPosition()); // Position of the spot light
    shader.SetVector3f("u_SpotLight.direction", m_Camera.GetForwardDirection()); // Direction of the spot light
    shader.SetUniform4f("u_SpotLight.ambient", 0.0f, 0.0f, 0.0f, 1.0f); // Ambient light color
    shader.SetUniform4f("u_SpotLight.diffuse", 1.0f, 1.0f, 1.0f, 1.0f); // Diffuse light color
    shader.SetUniform4f("u_SpotLight.specular", 1.0f, 1.0f, 1.0f, 1.0f); // Specular light color
    shader.SetUniformFloat("u_SpotLight.constant", pointLightAttenuationFactors.x);
    shader.SetUniformFloat("u_SpotLight.linear", pointLightAttenuationFactors.y);
    shader.SetUniformFloat("u_SpotLight.quadratic", pointLightAttenuationFactors.z);
    shader.SetUniformFloat("u_SpotLight.cutOff", glm::cos(glm::radians(5.0f))); // Inner cut-off angle for the spot light
    shader.SetUniformFloat("u_SpotLight.outerCutOff", glm::cos(glm::radians(17.5f))); // Outer cut-off angle for the spot light
}

void RenderMesh(const AssetLoader::Mesh& mesh, const Shader& shader, const glm::mat4& transform)
{
    shader.SetMatrix4f("u_Model", transform);
    mesh.Draw(shader);
}