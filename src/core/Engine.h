#pragma once
#include "Window.h"
#include "Renderer.h"
#include "../gui/GUIManager.h"
#include "../scene/Scene.h"

class Engine
{
public:
    Engine();
    ~Engine();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    Window *window;
    Renderer *renderer;
    GUIManager *guiManager;
    Scene *scene;
};
