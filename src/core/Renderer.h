#pragma once
#include "Window.h"
#include "../scene/Scene.h"

class Renderer
{
public:
    Renderer();
    ~Renderer();

    bool Initialize(Window *window);
    void Render(Scene *scene, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);

private:
    int width, height;
};
