#pragma once

#include <glm/glm.hpp>

#include <string>

class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void Use() const;

    void SetUniformBool(const std::string& name, bool value) const;
    void SetUniformInt(const std::string& name, int value) const;
    void SetUniformFloat(const std::string& name, float value) const;

    void SetUniform3f(const std::string& name, float v0, float v1, float v2) const;
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const;

	void SetVector2f(const std::string& name, const glm::vec2& value) const;
	void SetVector3f(const std::string& name, const glm::vec3& value) const;
	void SetVector4f(const std::string& name, const glm::vec4& value) const;

	void SetUniformMat4f(const std::string& name, const float* value) const;
	void SetMatrix4f(const std::string& name, const glm::mat4& matrix) const;
private:
    unsigned int CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
private:
    unsigned int m_ID;
};