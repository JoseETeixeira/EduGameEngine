#pragma once
#include "Window.h"
#include "../scene/Scene.h"

class Renderer
{
public:
    Renderer();
    ~Renderer();

    bool Initialize(Window *window);
    void Render(Scene *scene);

private:
    int width, height;
};
