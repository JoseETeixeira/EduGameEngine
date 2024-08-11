#pragma once
#include "../core/Window.h"

class GUIManager
{
public:
    GUIManager();
    ~GUIManager();

    bool Initialize(Window *window);
    void NewFrame(const float *viewMatrix, const float *projectionMatrix);
    void Render();
};