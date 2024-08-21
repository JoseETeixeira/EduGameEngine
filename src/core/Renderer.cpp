#include "Renderer.h"
#include <iostream>
#include <GL/glew.h>

Renderer::Renderer() {}

Renderer::~Renderer() {}

bool Renderer::Initialize(Window *window)
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    int width, height;
    glfwGetFramebufferSize(window->GetGLFWWindow(), &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST); // Ensure depth testing is enabled to handle grid and camera correctly

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    return true;
}

void Renderer::Render(Scene *scene, glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    // Debug: Check if the renderer is rendering the scene
    std::cout << "Renderer: Rendering Scene" << std::endl;

    // Ensure the depth test is enabled
    glEnable(GL_DEPTH_TEST);

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the scene
    scene->Draw(viewMatrix, projectionMatrix);

    // Check for any OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error: " << error << std::endl;
    }
}
