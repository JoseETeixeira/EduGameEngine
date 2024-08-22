// Scene.cpp

#include "Scene.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <entt.hpp>
#include "../ecs/MovementSystem.h"
#include "../ecs/NameComponent.h"

Scene::Scene(entt::registry *registry) : registry(registry)
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

    // Manually load the texture for testing
    std::string diffusePath = "C:/Users/pc/Downloads/Prunus_Pendula_OBJ/maps/prunus_pendula_branch_3_1_diffuse.jpg";
    Texture texture;
    texture.id = TextureFromFile(diffusePath.c_str());
    texture.type = "texture_diffuse";
    texture.path = diffusePath;
    if (texture.id == 0)
    {
        std::cerr << "Texture failed to load at path: " << diffusePath << std::endl;
    }
    else
    {
        textures.push_back(texture);
    }

    static entt::entity meshEntity = entt::null;
    bool initialized = false;
    if (!initialized)
    {
        meshEntity = registry->create();
        registry->emplace<TransformComponent>(meshEntity, glm::vec3(0.0f, 0.0f, 0.0f));
        registry->emplace<NameComponent>(meshEntity, "Mesh");
        initialized = true;
    }

    Mesh newMesh(vertices, indices, textures, registry, meshEntity);
    AddMesh(newMesh);
}

std::vector<Texture> Scene::LoadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        texture.id = TextureFromFile(str.C_Str());
        texture.type = typeName;
        texture.path = str.C_Str();
        textures.push_back(texture);
    }
    return textures;
}

GLuint Scene::TextureFromFile(const char *path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
