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
#include <iomanip>
#include "shader.h"

// Structure to hold vertex data including position and normal
struct Vertex {
    glm::vec3 Position; // Vertex position in 3D space
    glm::vec3 Normal;   // Vertex normal for lighting calculations
};

// Structure representing a mesh with vertices, indices, and OpenGL buffers
struct Mesh {
    std::vector<Vertex> vertices;          // Array of vertices
    std::vector<unsigned int> indices;    // Array of indices for indexed drawing
    unsigned int VAO, VBO, EBO;           // OpenGL buffer objects

    // Constructor initializes mesh with vertices and indices
    Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
        : vertices(vertices), indices(indices) {
        setupMesh();
    }

    // Sets up OpenGL buffers and vertex attributes
    void setupMesh() {
        // Generate vertex array object
        glGenVertexArrays(1, &VAO);
        // Generate vertex buffer object
        glGenBuffers(1, &VBO);
        // Generate element buffer object
        glGenBuffers(1, &EBO);

        // Bind VAO for setting up vertex attributes
        glBindVertexArray(VAO);

        // Bind and fill vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        if (!vertices.empty()) {
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                vertices.data(), GL_STATIC_DRAW);
        }

        // Bind and fill index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        if (!indices.empty()) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                indices.data(), GL_STATIC_DRAW);
        }

        // Set vertex attribute for position (location 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // Set vertex attribute for normal (location 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, Normal));

        // Unbind VAO
        glBindVertexArray(0);
    }

    // Draws the mesh using the provided shader
    void Draw(Shader& shader) const {
        if (vertices.empty() || indices.empty()) {
            return;
        }
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()),
            GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

// Class to represent a 3D model composed of multiple meshes
class Model {
    std::vector<Mesh> meshes; // Collection of meshes in the model

    // Loads model from file using Assimp
    void loadModel(const std::string& path) {
        Assimp::Importer importer;
        // Import model with specified processing flags
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate | aiProcess_FlipUVs |
            aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

        // Check for import errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            return;
        }

        // Process each mesh in the scene
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[i];
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;

            // Process vertices
            for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
                Vertex vertex;
                // Set vertex position
                vertex.Position = glm::vec3(mesh->mVertices[j].x,
                    mesh->mVertices[j].y,
                    mesh->mVertices[j].z);

                // Set vertex normal, use default if none exists or invalid
                if (mesh->mNormals) {
                    vertex.Normal = glm::vec3(mesh->mNormals[j].x,
                        mesh->mNormals[j].y,
                        mesh->mNormals[j].z);
                    if (glm::length(vertex.Normal) < 0.001f) {
                        vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    }
                }
                else {
                    vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }
                vertices.push_back(vertex);
            }

            // Process indices
            for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
                aiFace face = mesh->mFaces[j];
                for (unsigned int k = 0; k < face.mNumIndices; k++) {
                    indices.push_back(face.mIndices[k]);
                }
            }

            // Add mesh to model
            meshes.emplace_back(vertices, indices);
        }
    }

public:
    // Constructor loads model from file
    Model(const std::string& path) {
        loadModel(path);
    }

    // Draws all meshes in the model
    void Draw(Shader& shader) const {
        for (const auto& mesh : meshes) {
            mesh.Draw(shader);
        }
    }

    // Structure to hold bounding box information
    struct BoundingBox {
        glm::vec3 min; // Minimum coordinates
        glm::vec3 max; // Maximum coordinates
    };

    // Calculates the bounding box of the model
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

    // Saves the model to an OBJ file with applied transformation
    void saveToOBJ(const std::string& filename, const glm::mat4& transform) const {
        std::ofstream file(filename, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            return;
        }

        file << "# Generated OBJ file\n";
        file << std::fixed << std::setprecision(3);

        std::string buffer;
        buffer.reserve(1024 * 1024);
        unsigned int vertexOffset = 1;

        for (const auto& mesh : meshes) {
            // Write transformed vertices
            for (const auto& vertex : mesh.vertices) {
                glm::vec4 transformedPos = transform * glm::vec4(vertex.Position, 1.0f);
                buffer += "v " + std::to_string(transformedPos.x) + " " +
                    std::to_string(transformedPos.y) + " " +
                    std::to_string(transformedPos.z) + "\n";
            }

            // Write transformed normals
            for (const auto& vertex : mesh.vertices) {
                glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(transform)));
                glm::vec3 transformedNormal = glm::normalize(normalMatrix * vertex.Normal);
                buffer += "vn " + std::to_string(transformedNormal.x) + " " +
                    std::to_string(transformedNormal.y) + " " +
                    std::to_string(transformedNormal.z) + "\n";
            }

            // Write faces
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
    }
};

#endif