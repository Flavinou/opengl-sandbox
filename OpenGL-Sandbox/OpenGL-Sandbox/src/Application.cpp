#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image/stb_image.h>

// Settings 
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

// User input
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// Global uniforms
float u_Amount = 0.2f; // Amount of texture blending, can be set from user input or animation

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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "[ERROR]: Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    // Create shader
    Shader shader("resources/shaders/Vertex.glsl", "resources/shaders/Fragment.glsl");

    // Create texture
    Texture woodenTexture("resources/textures/wooden_container.jpg");
	woodenTexture.SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    woodenTexture.SetFilterMode(GL_NEAREST, GL_NEAREST);

	Texture awesomeFaceTexture("resources/textures/awesomeface.png");
    awesomeFaceTexture.SetFilterMode(GL_NEAREST, GL_NEAREST);

    // Renderer data
    float vertices[] =
	{   // positions            // colors            // texture coords
         0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,    0.55f, 0.55f, // top right
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,    0.55f, 0.45f, // bottom right
        -0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,    0.45f, 0.45f, // bottom left 
		-0.5f,  0.5f, 0.0f,     1.0f, 1.0f, 0.0f,    0.45f, 0.55f  // top left
    };
    unsigned int indices[] =
    {
        0, 1, 3, // first triangle
		1, 2, 3  // second triangle
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the vertex array first as container for the element and vertex buffer objects
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Configure vertex attributes (memory layout)
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
	// texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Handle user input
        processInput(window);

        // Rendering anything happens here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Wireframe mode
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// Bind the texture
		woodenTexture.Bind();
		awesomeFaceTexture.Bind(1); // Bind the second texture to texture unit 1

        // Bind the shader
        shader.Use();
         
        // Update the uniform color
        float timeValue = glfwGetTime();
        float colorValue = (sin(timeValue) / 2.0f) + 0.5f;
        shader.SetUniform4f("u_Color", colorValue, colorValue, colorValue, 1.0f);
		shader.SetUniformInt("u_Texture1", 0); // Set the first texture to texture unit 0
		shader.SetUniformInt("u_Texture2", 1); // Set the second texture to texture unit 1
		shader.SetUniformFloat("u_Amount", u_Amount);

		// Bind the vertex array object
        glBindVertexArray(VAO);

        // Render the geometry
        // glDrawArrays(GL_TRIANGLES, 0, 6); // solution when we draw each vertex one-by-one
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); // solution using index buffer and reuse the same set of vertices multiple times
        // glBindVertexArray(0); // no need to unbind VAO each time as there is only one at the moment

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Resource deallocation
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
		// Increase the amount of texture blending, clamped between 0.0 and 1.0
		u_Amount += 0.01f;
		if (u_Amount > 1.0f) u_Amount = 1.0f; // Clamp to 1.0
	}

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        // Decrease the amount of texture blending, clamped between 0.0 and 1.0
        u_Amount -= 0.01f;
        if (u_Amount < 0.0f) u_Amount = 0.0f; // Clamp to 0.0
    }
}