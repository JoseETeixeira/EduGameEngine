#pragma once
#include "../core/Window.h"
#include "../camera/camera.h"
// GUIManager.h
#include <string>
#include <vector>

class GUIManager
{
public:
    GUIManager();
    ~GUIManager();

    bool Initialize(Window *window);
    void NewFrame(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    void Render();
    void ProcessInput(GLFWwindow *window, float deltaTime);
    Camera camera;
    bool firstMouse;
    float lastX, lastY;

private:
    struct Asset
    {
        std::string name;
        unsigned int textureID;
    };

    std::vector<Asset> assets;
    std::string selectedAsset;

    bool isPlaying;

    void RenderEditorGUI(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    void RenderApplicationGUI();
    void RenderSources();
    void InitializeIcons();
    void RenderOutlinePanel();
    void ImportAsset();
    void AddAssetToScene(const std::string &assetPath);
    void CreateFolder();
    void CreatePlayerController();
    void RenderMainEditorPanel(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    void RenderDetailsPanel();
    void Render3DGrid(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
};
