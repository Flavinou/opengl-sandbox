#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_Position(position)
    , m_Yaw(yaw)
    , m_Pitch(pitch)
    , m_WorldUp(up)
{
    UpdateProjection();
}

void Camera::OnKeyPressed(float deltaTime, CameraMovement direction)
{
    float velocity = m_Speed * deltaTime;

    if (direction == CameraMovement::FORWARD)
        m_Position += velocity * m_Forward;
    if (direction == CameraMovement::BACKWARD)
        m_Position -= velocity * m_Forward;
    if (direction == CameraMovement::LEFT)
        m_Position -= velocity * m_Right;
    if (direction == CameraMovement::RIGHT)
        m_Position += velocity * m_Right;
}

void Camera::OnMouseMove(glm::vec2 offset, bool constrainPitch /*= true*/)
{
    offset.x *= m_Sensitivity;
    offset.y *= m_Sensitivity;

    m_Yaw += offset.x;
    m_Pitch += offset.y;

    if (constrainPitch)
    {
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;
    }

    UpdateProjection();
}

void Camera::OnMouseScroll(float yOffset)
{
    m_FOV -= (float)yOffset;
    if (m_FOV < 1.0f)
        m_FOV = 1.0f;
    if (m_FOV > 45.0f)
        m_FOV = 45.0f;
}

void Camera::UpdateProjection()
{
    glm::vec3 direction{};
    direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    direction.y = sin(glm::radians(m_Pitch));
    direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

    m_Forward = glm::normalize(direction);
    m_Right = glm::normalize(glm::cross(m_Forward, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
}
