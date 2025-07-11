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
    glDeleteFramebuffers(1, &m_DepthMapFramebuffer);

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
    GLCall(glEnable(GL_CULL_FACE));

    // Create shaders
    m_LitShader = std::make_shared<Shader>("resources/shaders/Vertex.glsl", "resources/shaders/LitFragment.glsl");
    m_UnlitShader = std::make_shared<Shader>("resources/shaders/Vertex.glsl", "resources/shaders/UnlitFragment.glsl");
    m_PointShadowMapShader = std::make_shared<Shader>("resources/shaders/PointShadowMappingVertex.glsl", "resources/shaders/PointShadowMappingFragment.glsl", "resources/shaders/PointShadowMappingGeometry.glsl");

    // Create model
    m_BackpackModel = std::make_shared<AssetLoader::Model>("resources/models/backpack/backpack.obj");

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

    // Create shadow / depth map framebuffer
    GLCall(glGenFramebuffers(1, &m_DepthMapFramebuffer));

    m_DepthCubemap = std::make_shared<Cubemap>(SHADOW_WIDTH, SHADOW_HEIGHT);
    m_DepthCubemap->SetData(nullptr, GL_DEPTH_COMPONENT);
    m_DepthCubemap->SetFilterMode(GL_NEAREST);
    m_DepthCubemap->SetWrapMode(GL_CLAMP_TO_EDGE);

    // Attach depth texture to shadow map framebuffer
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFramebuffer));
    GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemap->GetID(), 0));
    GLCall(glDrawBuffer(GL_NONE));
    GLCall(glReadBuffer(GL_NONE));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    // Bind the lit shader first to set the material shininess which is not meant to change
    m_LitShader->Use();
    m_LitShader->SetUniformFloat("u_Material.shininess", 64.0f);
    m_LitShader->SetUniformInt("u_Material.texture_diffuse1", 0);
    m_LitShader->SetUniformInt("u_Material.shadow_map1", 1);
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
    const float nearPlane = 1.0f, farPlane = 25.0f;
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, nearPlane, farPlane);
    glm::mat4 lightModel = glm::mat4(1.0f);

    m_PointShadowMapShader->Use();
    m_PointShadowMapShader->SetUniformFloat("u_FarPlane", farPlane);

    // Render the scene from the point of view of each light into the attached depth cubemap
    for (unsigned int i = 0; i < m_PointLightPositions.size(); i++)
    {
        // Move light position over time
        m_PointLightPositions[i].x = glm::sin(m_LastFrameTime) * 4.0f;
        m_PointLightPositions[i].z = glm::cos(m_LastFrameTime) * 3.0f;

        glm::mat4 pointLightModel = glm::mat4(1.0f);
        pointLightModel = glm::translate(pointLightModel, m_PointLightPositions[i]);
        pointLightModel = glm::scale(pointLightModel, glm::vec3(0.2f)); // Scale down the light source

        m_PointShadowMapShader->SetVector3f("u_LightPosition", m_PointLightPositions[i]);

        m_ShadowTransforms[0] = lightProjection *
            glm::lookAt(m_PointLightPositions[i], m_PointLightPositions[i] + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        m_ShadowTransforms[1] = lightProjection *
            glm::lookAt(m_PointLightPositions[i], m_PointLightPositions[i] + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        m_ShadowTransforms[2] = lightProjection *
            glm::lookAt(m_PointLightPositions[i], m_PointLightPositions[i] + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_ShadowTransforms[3] = lightProjection *
            glm::lookAt(m_PointLightPositions[i], m_PointLightPositions[i] + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        m_ShadowTransforms[4] = lightProjection *
            glm::lookAt(m_PointLightPositions[i], m_PointLightPositions[i] + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        m_ShadowTransforms[5] = lightProjection *
            glm::lookAt(m_PointLightPositions[i], m_PointLightPositions[i] + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

        for (int j = 0; j < 6; j++)
        {
            m_PointShadowMapShader->SetMatrix4f("u_ShadowMatrices[" + std::to_string(j) + "]", m_ShadowTransforms[j]);
        }

        GLCall(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFramebuffer));
        GLCall(glClear(GL_DEPTH_BUFFER_BIT));

        m_FloorTexture->Bind();

        // Draw the room cube
        GLCall(glDisable(GL_CULL_FACE));
        lightModel = glm::mat4(1.0f);
        lightModel = glm::scale(lightModel, glm::vec3(5.0f));
        RenderMesh(*m_LightSourceMesh, *m_PointShadowMapShader, lightModel);
        GLCall(glEnable(GL_CULL_FACE));

        // Draw the floor
        //m_PlaneMesh->Draw(*m_PointShadowMapShader);

        // Draw cubes (using bound wood texture)
        // "m_LightSourceMesh" is just a cube mesh scaled down to display the position of point lights
        glm::mat4 cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, glm::vec3(0.0f, 1.5f, 0.0f));
        cubeModel = glm::scale(cubeModel, glm::vec3(0.5f));
        RenderMesh(*m_LightSourceMesh, *m_PointShadowMapShader, cubeModel);

        cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, glm::vec3(2.0f, 0.0f, 1.0f));
        cubeModel = glm::scale(cubeModel, glm::vec3(0.5f));
        RenderMesh(*m_LightSourceMesh, *m_PointShadowMapShader, cubeModel);

        cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, glm::vec3(-1.0f, 0.0f, 2.0f));
        cubeModel = glm::rotate(cubeModel, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
        cubeModel = glm::scale(cubeModel, glm::vec3(0.25f));
        RenderMesh(*m_LightSourceMesh, *m_PointShadowMapShader, cubeModel);
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
    m_LitShader->SetVector3f("u_LightPosition", m_PointLightPositions[0]); // HACK HACK HACK
    m_LitShader->SetUniformFloat("u_FarPlane", farPlane);

    SetLightingUniforms(*m_LitShader);

    // Set the model, view and projection matrix uniforms
    glm::mat4 projection = glm::perspective(glm::radians(m_Camera.GetFOV()), (float)m_ViewportWidth / (float)m_ViewportHeight, 0.1f, 100.0f);
    glm::mat4 view = m_Camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    m_LitShader->SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
    m_LitShader->SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader

    m_FloorTexture->Bind();
    m_DepthCubemap->Bind(1);

    // Draw the room cube
    GLCall(glDisable(GL_CULL_FACE));
    m_LitShader->SetUniformInt("u_ReverseNormals", 1);
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f));
    RenderMesh(*m_LightSourceMesh, *m_LitShader, model);
    m_LitShader->SetUniformInt("u_ReverseNormals", 0);
    GLCall(glEnable(GL_CULL_FACE));

    // Draw the floor
    //m_PlaneMesh->Draw(*m_LitShader);

    // Draw cubes (using bound wood texture)
    // "m_LightSourceMesh" is just a cube mesh scaled down to display the position of point lights
    glm::mat4 cubeModel = glm::mat4(1.0f);
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
        lightModel = glm::translate(lightModel, m_PointLightPositions[i]);
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
        shader.SetVector3f(pointLightName + ".position", m_PointLightPositions[i]);
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