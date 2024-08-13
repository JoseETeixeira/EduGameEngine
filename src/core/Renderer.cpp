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

void Renderer::Render(Scene *scene)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render scene

    // Example: scene->Draw();
}
