#include "Engine.h"
#include "Window.h"            // Include Window's definition
#include "Renderer.h"          // Include Renderer definition if necessary
#include "../gui/GuiManager.h" // Include GUIManager definition if necessary
#include "../scene/Scene.h"    // Include Scene definition if necessary
#include <GLFW/glfw3.h>        // Ensure this is included to define GLFWwindow
#include "../ecs/MovementSystem.h"
#include <iostream>

Engine::Engine()
    : window(nullptr), renderer(nullptr), guiManager(nullptr), scene(nullptr), isPlaying(false)
{

    // Add the MovementSystem
    auto movementSystem = std::make_shared<MovementSystem>();
    systemManager.AddSystem(movementSystem);
}

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
    guiManager->engine = this;

    scene = new Scene();
    // Initialize scene, load resources, etc.

    return true;
}

void Engine::Run()
{
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!window->ShouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        guiManager->ProcessInput(window->GetGLFWWindow(), deltaTime);

        window->PollEvents();

        int width, height;
        glfwGetFramebufferSize(window->GetGLFWWindow(), &width, &height);

        glm::mat4 viewMatrix = guiManager->camera.GetViewMatrix();
        glm::mat4 projectionMatrix = guiManager->camera.GetProjectionMatrix(static_cast<float>(width), static_cast<float>(height));

        if (isPlaying)
        {
            systemManager.TickAllSystems(registry, deltaTime);
        }

        guiManager->NewFrame(viewMatrix, projectionMatrix);
        scene->Update();
        renderer->Render(scene);
        guiManager->Render(viewMatrix, projectionMatrix);

        window->SwapBuffers();
    }
}

void Engine::SetPlaying(bool play)
{
    isPlaying = play;
}

void Engine::Shutdown()
{
    delete scene;
    delete guiManager;
    delete renderer;
    delete window;
}