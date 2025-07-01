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

int main()
{
	Application* app = new Application(
        SCREEN_WIDTH, 
        SCREEN_HEIGHT, 
        Camera(glm::vec3(0.0f, 10.0f, 15.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -30.0f)
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
    delete[] m_ModelMatrices;
    glDeleteBuffers(1, &m_InstanceVBO);
    glDeleteBuffers(1, &m_AsteroidsInstanceVBO);

    // Cleanup GLFW resources
    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
	}

    glfwTerminate();
}

void Application::Initialize()
{
    glEnable(GL_DEPTH_TEST);

    // Create shaders
    m_LitShader = std::make_shared<Shader>("resources/shaders/Vertex.glsl", "resources/shaders/LitFragment.glsl");
    m_InstancedLitShader = std::make_shared<Shader>("resources/shaders/InstanceLitVertex.glsl", "resources/shaders/LitFragment.glsl");
    m_UnlitShader = std::make_shared<Shader>("resources/shaders/Vertex.glsl", "resources/shaders/UnlitFragment.glsl");
    m_InstancedQuadShader = std::make_shared<Shader>("resources/shaders/InstanceVertex.glsl", "resources/shaders/UnlitInstanceFragment.glsl");
    m_NormalsDisplayShader = std::make_shared<Shader>("resources/shaders/InstanceLitVertex.glsl", "resources/shaders/UnlitFragment.glsl", "resources/shaders/NormalDisplayGeometry.glsl");
    m_CubemapShader = std::make_shared<Shader>("resources/shaders/CubemapVertex.glsl", "resources/shaders/CubemapFragment.glsl");

    // Create models
    m_BackpackModel = std::make_shared<AssetLoader::Model>("resources/models/backpack/backpack.obj");
    m_PlanetModel = std::make_shared<AssetLoader::Model>("resources/models/planet/planet.obj");
    m_AsteroidModel = std::make_shared<AssetLoader::Model>("resources/models/rock/rock.obj");

    // Cubemap faces locations
    m_TextureFaces = {
        "resources/textures/skybox/nebula/right.png",
        "resources/textures/skybox/nebula/left.png",
        "resources/textures/skybox/nebula/top.png",
        "resources/textures/skybox/nebula/bottom.png",
        "resources/textures/skybox/nebula/front.png",
        "resources/textures/skybox/nebula/back.png",
    };

    // Create cubemap
    m_CubemapTexture = std::make_shared<Cubemap>(m_TextureFaces.data());

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

    m_PointLightPositions =
    {
        //glm::vec3(0.7f, 0.2f, 2.0f),
        //glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -2.5f),
        //glm::vec3(0.0f, 0.0f, -3.0f)
    };

	// Create meshes
    m_LightSourceMesh = std::make_shared<AssetLoader::Mesh>(cubeVertices, sizeof(cubeVertices) / sizeof(cubeVertices[0]), 8);

    // Bind the lit shader first to set the material shininess which is not meant to change
    m_LitShader->Use();
	m_LitShader->SetUniformFloat("u_Material.shininess", 32.0f);

    // Initialize a single quad vertices
    float quadVertices[] = 
    {
        // positions         // colors
        -0.05f,  0.05f,      1.0f, 0.0f, 0.0f, // top left
         0.05f, -0.05f,      0.0f, 1.0f, 0.0f, // bottom right
        -0.05f, -0.05f,      0.0f, 0.0f, 1.0f, // bottom left

        -0.05f,  0.05f,      1.0f, 0.0f, 0.0f, // top left
         0.05f, -0.05f,      0.0f, 1.0f, 0.0f, // bottom right
         0.05f,  0.05f,      0.0f, 1.0f, 1.0f, // top right
    };

    // Cubemap vertices
    float skyboxVertices[] =
    {
        // positions            // normals
        -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f
    };

    const int stride = 6 * sizeof(float);
    int size = sizeof(skyboxVertices);
    int count = size / stride;
    m_CubemapMesh = std::make_shared<AssetLoader::SimpleMesh>(skyboxVertices, size, count);
    m_CubemapMesh->SetVertexAttribute(0, 3, GL_FLOAT, false, stride, nullptr);
    m_CubemapMesh->SetVertexAttribute(1, 3, GL_FLOAT, false, stride, (void*)(3 * sizeof(float)));

    // Initialize an array containing offsets for the quad to be rendered in instances
    //int index = 0;
    //float offset = 0.1f;
    //for (int y = -10; y < 10; y += 2)
    //{
    //    for (int x = -10; x < 10; x += 2)
    //    {
    //        m_InstancedOffsets[index++] = glm::vec2((float)x / 10.0f + offset, (float)y / 10.0f + offset);
    //    }
    //}

    //// Set the instanced quad shader "offsets" uniform
    //m_InstancedQuadShader->Use();
    //for (unsigned int i = 0; i < sizeof(m_InstancedOffsets) / sizeof(m_InstancedOffsets[0]); i++)
    //{
    //    m_InstancedQuadShader->SetVector2f(("u_Offsets[" + std::to_string(i) + "]"), m_InstancedOffsets[i]);
    //}

    // Create simple mesh from quad
    //const int stride = 5 * sizeof(float);
    //int size = sizeof(quadVertices);
    //int count = size / stride;
    //m_QuadMesh = std::make_shared<AssetLoader::SimpleMesh>(quadVertices, size, count);
    //m_QuadMesh->SetVertexAttribute(0, 2, GL_FLOAT, false, stride, (void*)0); // Position attribute
    //m_QuadMesh->SetVertexAttribute(1, 3, GL_FLOAT, false, stride, (void*)(2 * sizeof(float))); // Color attribute

    //// Initialize instanced array vertex buffer (tells the GPU that we're going to add extra data to vertices) and send it to the GPU 
    //// (allows us to send way more data than by using uniforms which have got an hardware implementation limit)
    //int offsetsLength = sizeof(m_InstancedOffsets) / sizeof(m_InstancedOffsets[0]);
    //m_QuadMesh->AddVertexBuffer(m_InstanceVBO);
    //m_QuadMesh->SetVertexBufferData(&m_InstancedOffsets[0], sizeof(glm::vec2) * offsetsLength);

    //m_QuadMesh->SetVertexAttribute(2, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    //m_QuadMesh->UnbindVertexBuffer();
    //m_QuadMesh->SetVertexAttributeInstanceRate(2, 1); // Update the content of the vertex attribute per instance instead of per vertex (every 6 vertices in this case)

    // Generate a large list of semi-random model transformation matrices
    m_ModelMatrices = new glm::mat4[ASTEROIDS_AMOUNT];
    std::srand(m_LastFrameTime); // Initialize random seed from the current frame time
    const float radius = 150.0f;
    float offset = 25.0f;
    for (unsigned int i = 0; i < ASTEROIDS_AMOUNT; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);

        // Translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)ASTEROIDS_AMOUNT * 360.0f; // 360 * [0; 1] - giving the full range of angles
        float displacement = (std::rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = std::sin(angle) * radius + displacement;
        displacement = (std::rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
        displacement = (std::rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = std::cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // Scale: scale between 0.05f and 0.25f the original model
        float scale = static_cast<float>((std::rand() % 20) / 100.0f + 0.05f);
        model = glm::scale(model, glm::vec3(scale));

        // Rotation: add random rotation around a (semi-) randomly picked rotation axis vector
        float rotation = static_cast<float>((std::rand() % 360));
        model = glm::rotate(model, rotation, glm::vec3(0.4f, 0.6f, 0.8f));

        // Add the final matrix to the array
        m_ModelMatrices[i] = model;
    }

    // Hmm bad API... vertex arrays / buffers are not intrinsically tied to anything but the GPU
    // Attaching it to a model class feels bad, man
    m_AsteroidModel->AddVertexBuffer(m_AsteroidsInstanceVBO);
    m_AsteroidModel->SetVertexBufferData(&m_ModelMatrices[0], sizeof(glm::mat4) * ASTEROIDS_AMOUNT);

    // Set vertex attributes for each asteroid mesh
    auto& asteroidMeshes = m_AsteroidModel->GetMeshes();
    for (unsigned int i = 0; i < asteroidMeshes.size(); i++)
    {
        m_AsteroidModel->BindVertexArray(asteroidMeshes[i]->GetVAO());

        m_AsteroidModel->SetVertexAttribute(3, 4, GL_FLOAT, false, sizeof(glm::mat4), (void*)0);
        m_AsteroidModel->SetVertexAttribute(4, 4, GL_FLOAT, false, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        m_AsteroidModel->SetVertexAttribute(5, 4, GL_FLOAT, false, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        m_AsteroidModel->SetVertexAttribute(6, 4, GL_FLOAT, false, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        m_AsteroidModel->SetVertexAttributeInstanceRate(3, 1); // Update the content of the vertex attribute per instance instead of per vertex
        m_AsteroidModel->SetVertexAttributeInstanceRate(4, 1);
        m_AsteroidModel->SetVertexAttributeInstanceRate(5, 1);
        m_AsteroidModel->SetVertexAttributeInstanceRate(6, 1);

        m_AsteroidModel->UnbindVertexArray();
    }

    // Set the normal display color
    m_NormalsDisplayShader->Use();
    m_NormalsDisplayShader->SetUniform4f("u_Color", 0.0f, 1.0f, 0.0f, 1.0f); // Set the color for normal display

    m_CubemapShader->Use();
    m_CubemapShader->SetUniformFloat("u_Cubemap", 0);
}

void Application::Run()
{
    // Main rendering loop
    while (!glfwWindowShouldClose(m_Window))
    {
        m_FrameId++;

        float currentFrame = (float)glfwGetTime();
        m_DeltaTime = currentFrame - m_LastFrameTime;
        m_LastFrameTime = currentFrame;

        float frameTimeMillis = m_DeltaTime * 1000.0f;
        float framesPerSecond = 1 / m_DeltaTime;

        // Log frame time + current FPS every 10th frame
        if (m_FrameId % 10 == 0)
        {
            std::cout << "Frame time: " << std::to_string(frameTimeMillis) << "ms (" << std::to_string(framesPerSecond) << " fps)" << std::endl;
        }

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
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // Draw instanced quad
    //m_InstancedQuadShader->Use();
    //m_QuadMesh->DrawInstanced(100);

    m_LitShader->Use();

    m_LitShader->SetVector3f("u_ViewPosition", m_Camera.GetWorldPosition());

    SetLightingUniforms(*m_LitShader);

    //// Set the model, view and projection matrix uniforms
    glm::mat4 projection = glm::perspective(glm::radians(m_Camera.GetFOV()), (float)m_ViewportWidth / (float)m_ViewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = m_Camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    m_LitShader->SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
    m_LitShader->SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader

    // Draw planet
    model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
    m_LitShader->SetMatrix4f("u_Model", model); // Set the model matrix for the shader

    m_PlanetModel->Draw(*m_LitShader);

    // Draw asteroids
    m_InstancedLitShader->Use();
    m_InstancedLitShader->SetVector3f("u_ViewPosition", m_Camera.GetWorldPosition());

    SetLightingUniforms(*m_InstancedLitShader);

    // Set the model, view and projection matrix uniforms
    m_InstancedLitShader->SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
    m_InstancedLitShader->SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader

    // Draw planet
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
    m_InstancedLitShader->SetMatrix4f("u_Model", model); // Set the model matrix for the shader

    for (unsigned int i = 0; i < m_AsteroidModel->GetMeshes().size(); i++)
    {
        m_AsteroidModel->DrawInstanced(*m_InstancedLitShader, ASTEROIDS_AMOUNT);

        // Second pass, render the normals of the backpack model
        m_NormalsDisplayShader->Use();
        m_NormalsDisplayShader->SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
        m_NormalsDisplayShader->SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader
        m_NormalsDisplayShader->SetMatrix4f("u_Model", model); // Set the model matrix for the shader

        // Draw the normals of the backpack model
        m_AsteroidModel->DrawInstanced(*m_NormalsDisplayShader, ASTEROIDS_AMOUNT);
    }

    // Draw main scene (backpack model + point light orbiting around the model)
    //m_BackpackModel->Draw(*m_LitShader); // Draw the backpack model with the lit shader

    m_UnlitShader->Use();
    m_UnlitShader->SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
    m_UnlitShader->SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader

    // Calculate the point lights model matrices and render them
    for (unsigned int i = 0; i < m_PointLightPositions.size(); i++)
    {
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, { glm::sin(m_LastFrameTime) * m_PointLightPositions[i].x * 5.0f, m_PointLightPositions[i].y, glm::cos(m_LastFrameTime) * m_PointLightPositions[i].z * 5.0f });
        lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // Scale down the light source

        m_UnlitShader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
        m_UnlitShader->SetMatrix4f("u_Model", lightModel);

        // Render the light source model
        m_LightSourceMesh->Draw(*m_UnlitShader);
    }

    // Skybox pass - make sure to render last !
    glDepthFunc(GL_LEQUAL); // Disable writing to the depth buffer, why care about that ?
    m_CubemapShader->Use();

    // Set the view and projection matrix uniforms
    glm::mat4 skyboxProjection = glm::perspective(glm::radians(m_Camera.GetFOV()), (float)m_ViewportWidth / (float)m_ViewportHeight, 0.1f, 1001.0f);
    glm::mat4 skyboxView = m_Camera.GetViewMatrix();
    skyboxView = glm::mat4(glm::mat3(skyboxView));
    m_CubemapShader->SetMatrix4f("u_Projection", skyboxProjection);
    m_CubemapShader->SetMatrix4f("u_View", skyboxView);
    m_CubemapMesh->Draw();
    glDepthFunc(GL_LESS);
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
        shader.SetVector3f(pointLightName + ".position", { glm::sin(m_LastFrameTime) * m_PointLightPositions[i].x * 5.0f, m_PointLightPositions[i].y, glm::cos(m_LastFrameTime) * m_PointLightPositions[i].z * 5.0f });
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