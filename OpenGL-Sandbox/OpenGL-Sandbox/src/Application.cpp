#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Settings 
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

// User input
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// OpenGL Utilities
GLuint setupShaders(const std::string& vertexSourcePath, const std::string& fragmentSourcePath);

// File Utilities
std::string loadFile(const std::string& filePath);

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

    // Create shader program
    GLuint shaderProgram = setupShaders("resources/shaders/Vertex.glsl", "resources/shaders/Fragment.glsl");
    GLuint redShaderProgram = setupShaders("resources/shaders/Vertex.glsl", "resources/shaders/FragmentRed.glsl");

    // Renderer data
    float vertices_first_triangle[] =
    {
        -0.5f,  0.5f, 0.0f, // top
         0.0f, -0.5f, 0.0f, // bottom right
        -1.0f, -0.5f, 0.0f, // bottom left
    };

    float vertices_second_triangle[] =
    {
         0.5f,  0.5f, 0.0f, // top
         1.0f, -0.5f, 0.0f, // bottom right
         0.0f, -0.5f, 0.0f  // bottom left
    };

    unsigned int VAOs[2], VBOs[2];
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);

    // Bind the vertex array first as container for the element and vertex buffer objects
    glBindVertexArray(VAOs[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_first_triangle), vertices_first_triangle, GL_STATIC_DRAW);

    // Configure vertex attributes (memory layout)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAOs[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_second_triangle), vertices_second_triangle, GL_STATIC_DRAW);

    // Configure vertex attributes (memory layout)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

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

        glUseProgram(shaderProgram);
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3); // solution when we draw each vertex one-by-one        
        
        glUseProgram(redShaderProgram);
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLES, 0, 3); // solution when we draw each vertex one-by-one

        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); // solution using index buffer and reuse the same set of vertices multiple times
        // glBindVertexArray(0); // no need to unbind VAO each time as there is only one at the moment

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Resource deallocation
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteProgram(shaderProgram);

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

GLuint setupShaders(const std::string& vertexSourcePath, const std::string& fragmentSourcePath)
{
    // Vertex shader
    std::string vertexShaderSource = loadFile(vertexSourcePath);
    const char* vertSrc = vertexShaderSource.c_str();

    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertSrc, nullptr);
    glCompileShader(vertexShader);

    // Vertex shader compilation errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "[ERROR]: Vertex shader compilation failed: " << infoLog << std::endl;
    }

    // Fragment shader
    std::string fragmentShaderSource = loadFile(fragmentSourcePath);
    const char* fragSrc = fragmentShaderSource.c_str();

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragSrc, nullptr);
    glCompileShader(fragmentShader);

    // Fragment shader compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "[ERROR]: Fragment shader compilation failed: " << infoLog << std::endl;
    }

    // Link shaders together
    GLuint shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for shader linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "[ERROR]: Shaders linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

std::string loadFile(const std::string& filePath)
{
    std::ifstream fileStream(filePath, std::ifstream::in);

    if (!fileStream.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return {};
    }

    std::string line;
    std::stringstream outStream;
    while (std::getline(fileStream, line))
    {
        outStream << line << std::endl;
    }

    fileStream.close();

    return outStream.str();
}