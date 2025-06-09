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
    if (m_LockPitch)
        m_Pitch = 0.0f;

    if (m_LockRotation)
    {
        m_Yaw = -90.0f;
        m_Pitch = 0.0f;
    }

    glm::vec3 direction =
    {
        glm::cos(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch)),
        glm::sin(glm::radians(m_Pitch)),
        glm::sin(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch))
    };

    m_Forward = glm::normalize(direction);
    m_Right = glm::normalize(glm::cross(m_Forward, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
}

/*static */glm::mat4 Camera::LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
    glm::vec3 zAxis = glm::normalize(eye - center);
    glm::vec3 xAxis = glm::normalize(glm::cross(glm::normalize(up), zAxis));
    glm::vec3 yAxis = glm::cross(zAxis, xAxis);

    glm::mat4 rotation{ 1.0f }, translation{ 1.0f };

    translation[3][0] = -eye.x;
    translation[3][1] = -eye.y;
    translation[3][2] = -eye.z;

    rotation[0][0] = xAxis.x;
    rotation[1][0] = xAxis.y;
    rotation[2][0] = xAxis.z;
    rotation[0][1] = yAxis.x;
    rotation[1][1] = yAxis.y;
    rotation[2][1] = yAxis.z;
    rotation[0][2] = zAxis.x;
    rotation[1][2] = zAxis.y;
    rotation[2][2] = zAxis.z;

    return rotation * translation;
}
