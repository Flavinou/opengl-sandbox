#pragma once

#include <glm/glm.hpp>

#include <string>

class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void Use();

    void SetUniformBool(const std::string& name, bool value);
    void SetUniformInt(const std::string& name, int value);
    void SetUniformFloat(const std::string& name, float value);

    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);

	void SetVector2f(const std::string& name, const glm::vec2& value);
	void SetVector3f(const std::string& name, const glm::vec3& value);
	void SetVector4f(const std::string& name, const glm::vec4& value);

	void SetUniformMat4f(const std::string& name, const float* value);
	void SetMatrix4f(const std::string& name, const glm::mat4& matrix);
private:
    unsigned int CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
private:
    unsigned int m_ID;
};