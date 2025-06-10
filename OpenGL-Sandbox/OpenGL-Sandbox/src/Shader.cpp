#include "Shader.h"

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

namespace File
{
    static std::string Load(const std::string& filePath)
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
}

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexSource = File::Load(vertexPath);
    std::string fragmentSource = File::Load(fragmentPath);
    m_ID = CreateShader(vertexSource, fragmentSource);
}

Shader::~Shader()
{
    glDeleteProgram(m_ID);
}

void Shader::Use()
{
    glUseProgram(m_ID);
}

void Shader::SetUniformBool(const std::string& name, bool value)
{
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), (int)value);
}

void Shader::SetUniformInt(const std::string& name, int value)
{
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), value);
}

void Shader::SetUniformFloat(const std::string& name, float value)
{
    glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    glUniform3f(glGetUniformLocation(m_ID, name.c_str()), v0, v1, v2);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    glUniform4f(glGetUniformLocation(m_ID, name.c_str()), v0, v1, v2, v3);
}

void Shader::SetUniformMat4f(const std::string& name, const float* value)
{
	glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, value);
}

unsigned int Shader::CreateShader(const std::string& vertexSource, const std::string& fragmentSource)
{
    // Vertex shader
    const char* vertexStr = vertexSource.c_str();
    const char* fragmentStr = fragmentSource.c_str();

    unsigned int vertexShader, fragmentShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexStr, nullptr);
    glCompileShader(vertexShader);

    // Vertex shader compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "[ERROR]: Vertex shader compilation failed: " << infoLog << std::endl;
    }

    // Fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentStr, nullptr);
    glCompileShader(fragmentShader);

    // Fragment shader compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "[ERROR]: Fragment shader compilation failed: " << infoLog << std::endl;
    }

    // Link shaders together
    unsigned int shaderProgram;
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

