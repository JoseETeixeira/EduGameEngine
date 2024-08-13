#include "Engine.h"
#include "Window.h"            // Include Window's definition
#include "Renderer.h"          // Include Renderer definition if necessary
#include "../gui/GuiManager.h" // Include GUIManager definition if necessary
#include "../scene/Scene.h"    // Include Scene definition if necessary
#include <GLFW/glfw3.h>        // Ensure this is included to define GLFWwindow

Engine::Engine()
    : window(nullptr), renderer(nullptr), guiManager(nullptr), scene(nullptr) {}

Engine::~Engine()
{
    Shutdown();
}

bool Engine::Initialize()
{
    window = new Window(800, 600, "Edu Game Engine", false); // Set fullscreen to true
    if (!window->Initialize())
    {
        return false;
    }

    renderer = new Renderer();
    if (!renderer->Initialize(window))
    {
        return false;
    }

    guiManager = new GUIManager();
    if (!guiManager->Initialize(window))
    {
        return false;
    }

    scene = new Scene();
    // Initialize scene, load resources, etc.

    return true;
}

void Engine::Run()
{
    while (!window->ShouldClose())
    {
        window->PollEvents();

        // Retrieve the current width and height of the window
        int width, height;
        glfwGetFramebufferSize(window->GetGLFWWindow(), &width, &height);

        // Assuming GetViewMatrix and GetProjectionMatrix take arguments correctly
        glm::mat4 viewMatrix = guiManager->camera.GetViewMatrix();
        glm::mat4 projectionMatrix = guiManager->camera.GetProjectionMatrix(static_cast<float>(width), static_cast<float>(height));

        // Pass the view and projection matrices to the GUIManager's NewFrame function
        guiManager->NewFrame(viewMatrix, projectionMatrix);

        // Scene update and rendering
        scene->Update();
        renderer->Render(scene);
        guiManager->Render();

        window->SwapBuffers();
    }
}

void Engine::Shutdown()
{
    delete scene;
    delete guiManager;
    delete renderer;
    delete window;
}
