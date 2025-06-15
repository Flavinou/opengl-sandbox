#include "Camera.h"
#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>

// Settings 
const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 900;

// Camera system
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);

glm::vec2 lastMousePos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
bool firstMouse = true; // First time the mouse is captured by the window on focus

// Time step
float deltaTime = 0.0f; // Time betwwen current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// User input
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void process_input(GLFWwindow* window, float ts);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Window creation
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Sandbox", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window!" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Capture mouse by default when the application gets focus
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set window callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "[ERROR]: Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Create shader
    Shader litShader("resources/shaders/Vertex.glsl", "resources/shaders/SpotLightFragment.glsl");
    Shader unlitShader("resources/shaders/Vertex.glsl", "resources/shaders/UnlitFragment.glsl");

    // Load container texture
	Texture containerTexture("resources/textures/container2.png");
	Texture specularTexture("resources/textures/container2_specular.png");

    // Renderer data - the vertices below define a cube that is located at the center of the screen
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

        -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,     1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,     1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,     0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,     1.0f, 0.0f,

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

    // The array defines a new position at each index for a new cube to be drawn onto the screen
    glm::vec3 cubePositions[] =
    {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    // Light source position in the world
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Configure vertex attributes (memory layout)
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Configure vertex attributes for the light VAO (memory layout)
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    // Bind the shader once, it is the same here
    litShader.Use();

    litShader.SetUniformInt("u_Material.diffuse", 0);
	litShader.SetUniformInt("u_Material.specular", 1);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Handle user input
        process_input(window, deltaTime);

        // Rendering anything happens here
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Wireframe mode
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        litShader.Use();

        // Update the uniform color
		litShader.SetVector3f("u_Light.position", camera.GetWorldPosition());
        litShader.SetVector3f("u_Light.direction", camera.GetForwardDirection()); // Spot light direction
		litShader.SetUniformFloat("u_Light.cutoff", glm::cos(glm::radians(12.5f))); // Inner cut-off angle for the spot light
        litShader.SetUniformFloat("u_Light.outerCutoff", glm::cos(glm::radians(17.5f))); // Outer cut-off angle for the spot light
        litShader.SetVector3f("u_ViewPosition", camera.GetWorldPosition());

        litShader.SetUniform4f("u_Light.ambient", 0.2f, 0.2f, 0.2f, 1.0f);
        litShader.SetUniform4f("u_Light.diffuse", 0.5f, 0.5f, 0.5f, 1.0f);
        litShader.SetUniform4f("u_Light.specular", 1.0f, 1.0f, 1.0f, 1.0f);

		// We want the point light to cover a distance of 50 units, so we set the attenuation factors accordingly
		litShader.SetUniformFloat("u_Light.constant", 1.0f);
		litShader.SetUniformFloat("u_Light.linear", 0.09f);
		litShader.SetUniformFloat("u_Light.quadratic", 0.032f);

        litShader.SetUniformFloat("u_Material.shininess", 64.0f);

        // Set the model, view and projection matrix uniforms
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetFOV()), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        litShader.SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
        litShader.SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader
		litShader.SetMatrix4f("u_Model", model); // Set the model matrix for the shader

        containerTexture.Bind(0); // Bind the texture to texture unit 0
		specularTexture.Bind(1); // Bind the specular texture to texture unit 1

        glBindVertexArray(VAO);
        for (int i = 0; i < 10; i++)
        {
            // Calculate the model matrix for each cube and render it
            glm::mat4 cubeModel = glm::mat4(1.0f);
            cubeModel = glm::translate(cubeModel, cubePositions[i]);
            float angle = 20.0f * i;
            cubeModel = glm::rotate(cubeModel, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            litShader.SetMatrix4f("u_Model", cubeModel);

            // Render one cube
            glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// A lamp object is weird here as we are using a directional light
        //glm::mat4 lightModel = glm::mat4(1.0f);
        //lightModel = glm::translate(lightModel, lightPos);
        //lightModel = glm::scale(lightModel, glm::vec3(0.2f));
        //glm::mat4 lightProjection = glm::perspective(glm::radians(camera.GetFOV()), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 lightView = camera.GetViewMatrix();

        //unlitShader.Use();

        //unlitShader.SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);

        //unlitShader.SetMatrix4f("u_Model", lightModel);
        //unlitShader.SetMatrix4f("u_View", lightView); // Pass the camera view matrix to the shader
        //unlitShader.SetMatrix4f("u_Projection", lightProjection); // Send the projection matrix to the shader

        //// Render the light source model
        //glBindVertexArray(lightVAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Resource deallocation
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastMousePos = { (float)xPos, (float)yPos };
        firstMouse = false;
    }

    glm::vec2 offset = 
    { 
        xPos - lastMousePos.x, 
        lastMousePos.y - yPos // reversed since y-coordinates range from bottom to top
    };
    lastMousePos = { (float)xPos, (float)yPos };

    camera.OnMouseMove(offset);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    camera.OnMouseScroll(yOffset);
}

void process_input(GLFWwindow* window, float ts)
{
    const float cameraSpeed = 2.5f * ts;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.OnKeyPressed(ts, CameraMovement::FORWARD);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.OnKeyPressed(ts, CameraMovement::BACKWARD);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.OnKeyPressed(ts, CameraMovement::LEFT);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.OnKeyPressed(ts, CameraMovement::RIGHT);
}