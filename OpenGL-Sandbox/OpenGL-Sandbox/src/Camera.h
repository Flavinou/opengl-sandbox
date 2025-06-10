#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera
{
public:
    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

    const glm::vec3& GetWorldPosition() const { return m_Position; }
    const float GetFOV() const { return m_FOV; }
    const float GetYaw() const { return m_Yaw; }
    const float GetPitch() const { return m_Pitch; }

    const glm::mat4 GetViewMatrix() const { return glm::lookAt(m_Position, m_Position + m_Forward, m_Up); }

    void OnKeyPressed(float deltaTime, CameraMovement direction);
    void OnMouseMove(glm::vec2 offset, bool constrainPitch = true);
    void OnMouseScroll(float yOffset);
private:
    void UpdateProjection();
private:
    glm::vec3 m_Position;
    glm::vec3 m_Forward{ 0.0f, 0.0f, -1.0f }, m_Up, m_Right, m_WorldUp;

    float m_Yaw = -90.0f, m_Pitch = 0.0f;
    float m_Speed = 2.5f, m_Sensitivity = 0.1f, m_FOV = 45.0f;
};