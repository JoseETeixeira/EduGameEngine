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
    void RenderApplicationGUI();
    void RenderEditorGUI(const float *viewMatrix, const float *projectionMatrix);
    void AddAssetToScene(const std::string &assetPath);

private:
    bool isPlaying;
};