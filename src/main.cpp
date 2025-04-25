#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "shader.h"
#include "model.h"
#include "camera.h"
#include "hair_transform.h"
#include "ui.h"
#include "input.h"

#include <imgui.h>

// Function to check for OpenGL errors at a specific location
static void checkGLError(const std::string& location) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL Error at " << location << ": " << err << std::endl;
    }
}

// Function to verify if a file exists on disk
static bool checkFileExists(const std::string& path) {
    std::ifstream file(path);
    bool exists = file.good();
    if (!exists) {
        std::cout << "Cannot access file: " << path << std::endl;
    }
    return exists;
}

int main() {
    std::cout << "Current working directory: " << std::filesystem::current_path().string() << std::endl;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    const unsigned int SCR_WIDTH = 1280;
    const unsigned int SCR_HEIGHT = 720;
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HairOnBald", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD (load OpenGL function pointers)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    checkGLError("GLAD initialization");

    // Print OpenGL and GLSL version information
    const GLubyte* glVersion = glGetString(GL_VERSION);
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    if (glVersion == nullptr || glslVersion == nullptr) {
        std::cout << "Failed to retrieve OpenGL or GLSL version" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    std::cout << "OpenGL Version: " << glVersion << std::endl;
    std::cout << "GLSL Version: " << glslVersion << std::endl;

    // Basic OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    checkGLError("OpenGL setup");

    // Load and compile shaders
    std::string vertexPath = "shaders/vertex.glsl";
    std::string fragmentPath = "shaders/fragment.glsl";
    if (!checkFileExists(vertexPath) || !checkFileExists(fragmentPath)) {
        std::cout << "Shader file missing" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    Shader shader(vertexPath.c_str(), fragmentPath.c_str());
    if (shader.ID == 0) {
        std::cout << "Shader program failed to load or link" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    GLint success;
    glGetProgramiv(shader.ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(shader.ID, 1024, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Load 3D models
    std::string baldHeadPath = "models/bald_head.obj";
    if (!checkFileExists(baldHeadPath)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    Model baldHead(baldHeadPath.c_str());

    std::string initialHairPath = "models/hair_front.obj";
    if (!checkFileExists(initialHairPath)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    Model hair(initialHairPath.c_str());

    // Camera setup
    Camera camera(glm::vec3(0.0f, 0.5f, 5.0f));

    // Hair transformation setup
    HairTransform hairTransform;
    hairTransform.setModelPath(initialHairPath);

    // UI and rendering options
    bool wireframe = false;
    bool renderBald = true;
    bool renderHair = true;
    bool mouseLocked = true;

    // UI initialization
    UI ui(&wireframe, &renderBald, &renderHair, &mouseLocked, &hairTransform, &hair);
    ui.initialize(window);

    // Input manager setup
    InputManager inputManager(window, &camera, &hairTransform, &wireframe, &renderBald, &renderHair, &mouseLocked);
    inputManager.setupCallbacks();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Position and scale initialization
    auto baldBox = baldHead.getBoundingBox();
    auto hairBox = hair.getBoundingBox();
    float targetScale = 1.0f;
    hairTransform.reset(targetScale);
    hairTransform.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));

    std::cout << "Bald Box: min(" << baldBox.min.x << ", " << baldBox.min.y << ", " << baldBox.min.z << "), max("
        << baldBox.max.x << ", " << baldBox.max.y << ", " << baldBox.max.z << ")\n";
    std::cout << "Hair Box: min(" << hairBox.min.x << ", " << hairBox.min.y << ", " << hairBox.min.z << "), max("
        << hairBox.max.x << ", " << hairBox.max.y << ", " << hairBox.max.z << ")\n";
    std::cout << "Initial Hair Position: (" << hairTransform.getPosition().x << ", "
        << hairTransform.getPosition().y << ", " << hairTransform.getPosition().z << ")\n";

    // Lighting setup
    glm::vec3 lightPos(2.0f, 2.0f, 5.0f);
    glm::vec3 lightColor(1.5f, 1.5f, 1.5f);

    // Frame timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // --- Main Rendering Loop ---
    while (!glfwWindowShouldClose(window)) {
        // Calculate frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Handle user input
        inputManager.processInput(deltaTime);

        // Start new ImGui frame
        ui.newFrame();
        ImGuiIO& io = ImGui::GetIO();

        // Mouse movement and scroll handling (if mouse is locked)
        if (mouseLocked) {
            float xoffset = io.MouseDelta.x;
            float yoffset = -io.MouseDelta.y;
            if (xoffset != 0.0f || yoffset != 0.0f) {
                camera.processMouseMovement(xoffset, yoffset);
            }
            if (io.MouseWheel != 0.0f) {
                camera.processMouseScroll(io.MouseWheel);
            }
        }

        // Render ImGui controls
        ui.renderUI(deltaTime);

        // Clear frame buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup camera matrices and light uniforms
        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.getFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.getPosition());
        shader.setVec3("lightColor", lightColor);

        // Draw bald head model
        if (renderBald) {
            glm::mat4 baldModel = glm::mat4(1.0f);
            baldModel = glm::translate(baldModel, glm::vec3(0.0f, 0.0f, 0.0f));
            baldModel = glm::scale(baldModel, glm::vec3(targetScale));
            shader.setMat4("model", baldModel);
            shader.setVec3("objectColor", glm::vec3(1.0f, 0.9f, 0.7f));
            baldHead.Draw(shader);
            checkGLError("Bald head render");
        }

        // Draw hair model
        if (renderHair) {
            glm::mat4 hairModelMatrix = hairTransform.getModelMatrix();
            shader.setMat4("model", hairModelMatrix);
            shader.setVec3("objectColor", hairTransform.getColor());
            hair.Draw(shader);
            checkGLError("Hair render");
        }

        // Finalize ImGui and swap buffers
        ui.renderEndFrame();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup resources
    ui.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
