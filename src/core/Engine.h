#pragma once
#include "Window.h"
#include "Renderer.h"
#include "../gui/GUIManager.h"
#include "../scene/Scene.h"
#include "../ecs/SystemManager.h"
#include "../ecs/MovementSystem.h"
#include <entt.hpp>

class GUIManager;

class Engine
{
public:
    Engine();
    ~Engine();

    bool Initialize();
    void Run();
    void Shutdown();
    void SetPlaying(bool play);

    entt::registry registry;

private:
    Window *window;
    Renderer *renderer;
    GUIManager *guiManager;
    Scene *scene;
    SystemManager systemManager;
    bool isPlaying;
};
