#pragma once
#include "../core/Engine.h"
#include "../core/Window.h"
#include "../camera/camera.h"
#include "../core/Renderer.h"
#include "../scene/Scene.h"
#include "../ecs/MovementSystem.h"
// GUIManager.h
#include <string>
#include <vector>
#include <glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <entt.hpp>

class Engine;

enum class TransformMode
{
    TRANSLATE,
    ROTATE,
    SCALE
};

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
    Engine *engine;
    Scene *scene;
    Renderer *renderer;
    entt::registry *registry;
    TransformMode currentTransformMode;
    bool objectSelected;
    glm::mat4 selectedObjectTransform;        // Store the transform of the selected object
    entt::entity selectedEntity = entt::null; // Track the selected entity

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
    void OpenProject();
    void CreateNewProject();
    void SaveProject();
    void LoadScene(const std::string &scenePath);
    bool contentDrawerOpen;
    void RenderTransformButtons();                                              // Add this method
    void ApplyTransformation(glm::mat4 viewMatrix, glm::mat4 projectionMatrix); // Add this method
    void RenderEntityNode(entt::entity entity, TransformComponent &transform);
};
