#pragma once
#include "../core/Window.h"
// GUIManager.h
#include <string>
#include <vector>

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
    void CreateFolder();
    void CreatePlayerController();

private:
    struct Asset
    {
        std::string name;
        unsigned int textureID;
    };

    std::vector<Asset> assets;
    std::string selectedAsset;

    bool isPlaying;

    void RenderEditorGUI(const float *viewMatrix, const float *projectionMatrix);
    void RenderApplicationGUI();
    void RenderSources();
    void InitializeIcons();
};
