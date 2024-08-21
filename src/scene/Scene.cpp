// Scene.cpp

#include "Scene.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

Scene::Scene()
{
    // Initialize game objects, load resources, etc.
}

Scene::~Scene()
{
    // Cleanup resources
}

void Scene::Update()
{
    // Update game objects, physics, etc.
}

void Scene::Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    // Debug: Ensure this method is called
    std::cout << "Scene: Drawing " << meshes.size() << " meshes" << std::endl;

    // Draw all meshes in the scene
    for (Mesh &mesh : meshes)
    {
        mesh.Draw(viewMatrix, projectionMatrix);
    }
}

void Scene::AddMesh(const Mesh &mesh)
{
    meshes.push_back(mesh);
}

void Scene::ProcessAssimpScene(const aiScene *scene)
{
    // Recursively process all nodes
    ProcessNode(scene->mRootNode, scene);
}

void Scene::ProcessNode(aiNode *node, const aiScene *scene)
{
    // Process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene);
    }

    // Then process all the children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

void Scene::ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (mesh->mNormals)
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // In the future, you could handle materials/textures here

    Mesh newMesh(vertices, indices, textures);
    AddMesh(newMesh);
}
