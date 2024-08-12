#pragma once
#include "../core/Window.h"
#include <string>

class GUIManager
{
public:
    GUIManager();
    ~GUIManager();

    bool Initialize(Window *window);
    void NewFrame(const float *viewMatrix, const float *projectionMatrix);
    void Render();
    void ImportAsset();
    void AddAssetToScene(const std::string &assetPath);
};