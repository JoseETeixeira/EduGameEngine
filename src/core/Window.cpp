#include "Window.h"
#include <iostream>

Window::Window(int width, int height, const char *title)
    : width(width), height(height), title(title), window(nullptr) {}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::Initialize()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    return true;
}

void Window::PollEvents()
{
    glfwPollEvents();
}

void Window::SwapBuffers()
{
    glfwSwapBuffers(window);
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(window);
}

GLFWwindow *Window::GetGLFWWindow() const
{
    return window;
}
