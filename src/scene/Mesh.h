#pragma once

#include <glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <string>

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture
{
    GLuint id;
    std::string type;
    std::string path;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const std::vector<Texture> &textures);
    void Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);

private:
    GLuint VAO, VBO, EBO;
    void SetupMesh();
};
