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
    return glm::perspective(glm::radians(Zoom), (float)width / (float)height, 0.1f, 100.0f);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    std::cout << "Initial Camera Position: " << glm::to_string(Position) << std::endl;
    float velocity = MovementSpeed * deltaTime;
    std::cout << "Velocity: " << velocity << std::endl;

    if (direction == FORWARD)
    {
        std::cout << "Moving FORWARD" << std::endl;
        Position += Front * velocity;
    }
    if (direction == BACKWARD)
    {
        std::cout << "Moving BACKWARD" << std::endl;
        Position -= Front * velocity;
    }
    if (direction == LEFT)
    {
        std::cout << "Moving LEFT" << std::endl;
        Position -= Right * velocity;
    }
    if (direction == RIGHT)
    {
        std::cout << "Moving RIGHT" << std::endl;
        Position += Right * velocity;
    }
    if (direction == UP)
    {
        std::cout << "Moving UP" << std::endl;
        Position += Up * velocity;
    }
    if (direction == DOWN)
    {
        std::cout << "Moving DOWN" << std::endl;
        Position -= Up * velocity;
    }

    std::cout << "Updated Camera Position: " << glm::to_string(Position) << std::endl;
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

    std::cout << "Yaw: " << Yaw << ", Pitch: " << Pitch << std::endl;
    std::cout << "Camera Front: " << glm::to_string(Front) << std::endl;
}

void Camera::updateCameraVectors()
{
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    // Also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp)); // Normalize the vectors
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}
