#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>
#include <iostream>

#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix(float width, float height) const
{
    return glm::perspective(glm::radians(Zoom), width / height, 0.1f, 1000.0f);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
    {
        Position += Front * velocity;
        std::cout << "Moving Forward" << std::endl;
    }
    if (direction == BACKWARD)
    {
        Position -= Front * velocity;
        std::cout << "Moving Backward" << std::endl;
    }
    if (direction == LEFT)
    {
        Position -= Right * velocity;
        std::cout << "Moving Left" << std::endl;
    }
    if (direction == RIGHT)
    {
        Position += Right * velocity;
        std::cout << "Moving Right" << std::endl;
    }
    if (direction == UP)
    {
        Position += Up * velocity;
        std::cout << "Moving Up" << std::endl;
    }
    if (direction == DOWN)
    {
        Position -= Up * velocity;
        std::cout << "Moving Down" << std::endl;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    // Optimized calculation and normalization
    Front = glm::normalize(glm::vec3(
        cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)),
        sin(glm::radians(Pitch)),
        sin(glm::radians(Yaw)) * cos(glm::radians(Pitch))));

    // Recalculate Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));

    std::cout << "Camera Position: " << glm::to_string(Position) << std::endl;
    std::cout << "Camera Front: " << glm::to_string(Front) << std::endl;
    std::cout << "Camera Up: " << glm::to_string(Up) << std::endl;
    std::cout << "Camera Right: " << glm::to_string(Right) << std::endl;
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}
