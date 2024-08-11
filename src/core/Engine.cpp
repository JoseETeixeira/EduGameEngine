#include "Engine.h"

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
    // Assuming you have viewMatrix and projectionMatrix defined and initialized in the Engine class
    float viewMatrix[16];       // This should be initialized based on your camera setup
    float projectionMatrix[16]; // This should be initialized based on your camera setup

    while (!window->ShouldClose())
    {
        window->PollEvents();

        // Update matrices if necessary before each frame
        // viewMatrix and projectionMatrix should be updated according to your camera or rendering setup

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
