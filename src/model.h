#ifndef MODEL_H
#define MODEL_H
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include "shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;
    Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) : vertices(vertices), indices(indices) {
        std::cout << "Mesh created with " << vertices.size() << " vertices and " << indices.size() << " indices" << std::endl;
        setupMesh();
    }
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        if (vertices.empty()) {
            std::cout << "Warning: Vertex buffer is empty" << std::endl;
        }
        else {
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(vertices.size()) * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        if (indices.empty()) {
            std::cout << "Warning: Index buffer is empty" << std::endl;
        }
        else {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizei>(indices.size()) * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        }
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glBindVertexArray(0);
    }
    void Draw(Shader& shader) const {
        if (vertices.empty() || indices.empty()) {
            std::cout << "Cannot draw mesh: empty vertices or indices" << std::endl;
            return;
        }
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

class Model {
    std::vector<Mesh> meshes;

    void loadModel(const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        std::cout << "Model has " << scene->mNumMeshes << " meshes" << std::endl;
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[i];
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
            for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
                Vertex vertex;
                vertex.Position = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
                if (mesh->mNormals) {
                    vertex.Normal = glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
                    if (glm::length(vertex.Normal) < 0.001f) {
                        vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                        std::cout << "Warning: Normal at vertex " << j << " is zero, using default (0,1,0)" << std::endl;
                    }
                }
                else {
                    vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    std::cout << "Warning: Mesh " << i << " has no normals, using default (0,1,0)" << std::endl;
                }
                vertices.push_back(vertex);
            }
            for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
                aiFace face = mesh->mFaces[j];
                for (unsigned int k = 0; k < face.mNumIndices; k++)
                    indices.push_back(face.mIndices[k]);
            }
            std::cout << "Mesh " << i << ": " << vertices.size() << " vertices, " << indices.size() << " indices" << std::endl;
            meshes.emplace_back(vertices, indices);
        }
    }

public:
    Model(const std::string& path) { loadModel(path); }
    void Draw(Shader& shader) const {
        for (const auto& mesh : meshes) mesh.Draw(shader);
    }
    struct BoundingBox {
        glm::vec3 min;
        glm::vec3 max;
    };
    BoundingBox getBoundingBox() const {
        BoundingBox box;
        box.min = glm::vec3(std::numeric_limits<float>::max());
        box.max = glm::vec3(std::numeric_limits<float>::lowest());
        for (const auto& mesh : meshes) {
            for (const auto& vertex : mesh.vertices) {
                box.min = glm::min(box.min, vertex.Position);
                box.max = glm::max(box.max, vertex.Position);
            }
        }
        return box;
    }

    void saveToOBJ(const std::string& filename, const glm::mat4& transform) const {
        std::ofstream file(filename, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Failed to open file for writing: " << filename << std::endl;
            return;
        }
        file << "# Generated OBJ file\n";
        file << std::fixed << std::setprecision(3);

        std::string buffer;
        buffer.reserve(1024 * 1024);
        unsigned int vertexOffset = 1;
        for (const auto& mesh : meshes) {
            for (const auto& vertex : mesh.vertices) {
                glm::vec4 transformedPos = transform * glm::vec4(vertex.Position, 1.0f);
                buffer += "v " + std::to_string(transformedPos.x) + " " +
                    std::to_string(transformedPos.y) + " " +
                    std::to_string(transformedPos.z) + "\n";
            }
            for (const auto& vertex : mesh.vertices) {
                glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(transform)));
                glm::vec3 transformedNormal = glm::normalize(normalMatrix * vertex.Normal);
                buffer += "vn " + std::to_string(transformedNormal.x) + " " +
                    std::to_string(transformedNormal.y) + " " +
                    std::to_string(transformedNormal.z) + "\n";
            }
            for (size_t i = 0; i < mesh.indices.size(); i += 3) {
                buffer += "f " + std::to_string(mesh.indices[i] + vertexOffset) + "//" +
                    std::to_string(mesh.indices[i] + vertexOffset) + " " +
                    std::to_string(mesh.indices[i + 1] + vertexOffset) + "//" +
                    std::to_string(mesh.indices[i + 1] + vertexOffset) + " " +
                    std::to_string(mesh.indices[i + 2] + vertexOffset) + "//" +
                    std::to_string(mesh.indices[i + 2] + vertexOffset) + "\n";
            }
            vertexOffset += static_cast<unsigned int>(mesh.vertices.size());
        }
        file.write(buffer.c_str(), buffer.size());
        file.close();
        std::cout << "Model saved to " << filename << std::endl;
    }
};

#endif