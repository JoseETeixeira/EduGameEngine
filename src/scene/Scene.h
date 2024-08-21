// Scene.h

#pragma once

#include <vector>
#include <string>
#include <glm.hpp>
#include <assimp/scene.h>
#include "Mesh.h" // Assuming you have a Mesh class to handle the rendering

class Scene
{
public:
    Scene();
    ~Scene();

    void Update();
    void Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix); // Pass view and projection matrices
    void AddMesh(const Mesh &mesh);

    void ProcessAssimpScene(const aiScene *scene);
    void ProcessNode(aiNode *node, const aiScene *scene);
    void ProcessMesh(aiMesh *mesh, const aiScene *scene);

private:
    std::vector<Mesh> meshes; // Store all meshes in the scene
};
