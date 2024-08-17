#pragma once
#include "../core/Engine.h"
#include "../core/Window.h"
#include "../camera/camera.h"
// GUIManager.h
#include <string>
#include <vector>
#include <glm.hpp>

class Engine;

class GUIManager
{
public:
    GUIManager();
    ~GUIManager();

    bool Initialize(Window *window);
    void NewFrame(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    void Render(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    void ProcessInput(GLFWwindow *window, float deltaTime);
    Camera camera;
    bool firstMouse;
    float lastX, lastY;
    void RenderTestCube(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    Engine *engine;

private:
    struct Asset
    {
        std::string name;
        unsigned int textureID;
    };

    std::vector<Asset> assets;
    std::string selectedAsset;

    bool isPlaying;
    GLint viewport[4];
    GLint scissorBox[4];
    int screenWidth;
    int screenHeight;
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
