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
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

// User input
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

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
	Texture awesomeFaceTexture("resources/textures/awesomeface.png");

    // Renderer data
    float vertices[] =
	{   // positions            // colors            // texture coords
         0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,    1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,    0.0f, 0.0f, // bottom left 
		-0.5f,  0.5f, 0.0f,     1.0f, 1.0f, 0.0f,    0.0f, 1.0f  // top left
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
        float time = glfwGetTime();
        float timeValue = (sin(time) / 2.0f) + 0.5f;

        // Transformation experiments
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::rotate(transform, time, glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
		// Matrices operations are applied in reverse order, so the translation happens first, then the rotation
		// resulting in a rotation around the point (0.5, -0.5) instead of the origin (thanks Copilot for making me gain time when typing this !)

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
        shader.SetUniform4f("u_Color", timeValue, timeValue, timeValue, 1.0f);
		shader.SetUniformInt("u_Texture1", 0); // Set the first texture to texture unit 0
		shader.SetUniformInt("u_Texture2", 1); // Set the second texture to texture unit 1
		shader.SetUniformMat4f("u_Transform", glm::value_ptr(transform)); // Pass the transformation matrix to the shader

		// Bind the vertex array object
        glBindVertexArray(VAO);

        // Render the geometry
        // glDrawArrays(GL_TRIANGLES, 0, 6); // solution when we draw each vertex one-by-one
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); // solution using index buffer and reuse the same set of vertices multiple times

        transform = glm::mat4(1.0f);
        transform = glm::scale(transform, glm::vec3(timeValue));
        transform = glm::translate(transform, glm::vec3(-0.5f, 0.5f, 0.0f));

		shader.SetUniformMat4f("u_Transform", glm::value_ptr(transform)); // Pass the transformation matrix to the shader

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
}