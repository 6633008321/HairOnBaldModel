#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "shader.h"
#include "model.h"
#include "ImGuiFileDialog.h"

// Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f, pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
float fov = 60.0f;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Wireframe mode
bool wireframe = false;

// Render controls
bool renderBald = true;
bool renderHair = true;
bool showSaveConfirmation = false;

// Debounce for input
float lastKeyPressTime = 0.0f;
const float keyCooldown = 0.2f;

// Hair transformation variables
glm::vec3 hairPosition(0.0f, 0.0f, 0.0f);
float hairScaleValue = 0.5f;
float hairRotation = 0.0f;
float hairRotationX = 0.0f;
float hairRotationZ = 0.0f;

// Mouse state
bool mouseLocked = true;

// Current hair model path
std::string currentHairPath = "models/hair_front.obj";
glm::vec3 hairColor = glm::vec3(0.5f, 0.3f, 0.2f); // Initial hair color (same as default)
float adjustSpeed = 0.5f; // Speed for position adjustments per second
float scaleSpeed = 0.05f; // Speed for scale adjustments per second
float rotationSpeed = 5.0f; // Speed for rotation adjustments per second
// Function to check OpenGL errors
static void checkGLError(const std::string& location) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL Error at " << location << ": " << err << std::endl;
    }
}

// Function to check if file exists
static bool checkFileExists(const std::string& path) {
    std::ifstream file(path);
    bool exists = file.good();
    if (!exists) {
        std::cout << "Cannot access file: " << path << std::endl;
    }
    return exists;
}

// Mouse callback
static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!mouseLocked) return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Scroll callback (zoom)
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= static_cast<float>(yoffset);
    if (fov < 30.0f) fov = 30.0f;
    if (fov > 90.0f) fov = 90.0f;
}

// Framebuffer size callback
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Input processing
static void processInput(GLFWwindow* window, Model& hair) {
    float currentTime = static_cast<float>(glfwGetTime());
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        lastKeyPressTime = currentTime;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
        renderBald = true;
        renderHair = false;
        std::cout << "Rendering bald head only" << std::endl;
        lastKeyPressTime = currentTime;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
        renderBald = false;
        renderHair = true;
        std::cout << "Rendering hair only" << std::endl;
        lastKeyPressTime = currentTime;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
        renderBald = true;
        renderHair = true;
        std::cout << "Rendering both" << std::endl;
        lastKeyPressTime = currentTime;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
        mouseLocked = !mouseLocked;
        glfwSetInputMode(window, GLFW_CURSOR, mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        lastKeyPressTime = currentTime;
        firstMouse = true;
        std::cout << "Mouse " << (mouseLocked ? "locked" : "unlocked") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
        IGFD::FileDialogConfig config;
        config.path = "models/";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseHairDlgKey", "Select Hair Model", ".obj", config);
        lastKeyPressTime = currentTime;
    }
    float transformSpeed = 0.1f * deltaTime;
    float scaleSpeed = 0.01f * deltaTime;
    float rotationSpeed = 10.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        hairPosition.y += transformSpeed;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        hairPosition.y -= transformSpeed;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        hairPosition.x -= transformSpeed;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        hairPosition.x += transformSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        hairRotation += rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        hairRotation -= rotationSpeed;
}

int main() {
    std::cout << "Current working directory: " << std::filesystem::current_path().string() << std::endl;

    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "HairOnBald", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!glfwGetCurrentContext()) {
        std::cout << "Failed to make GLFW context current" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    checkGLError("GLAD initialization");

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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, 1280, 720);
    checkGLError("OpenGL setup");

    std::string vertexPath = "shaders/vertex.glsl";
    std::string fragmentPath = "shaders/fragment.glsl";
    if (!checkFileExists(vertexPath) || !checkFileExists(fragmentPath)) {
        std::cout << "Shader file missing: " << vertexPath << " or " << fragmentPath << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "Loading shaders..." << std::endl;
    Shader shader(vertexPath.c_str(), fragmentPath.c_str());
    if (shader.ID == 0) {
        std::cout << "Shader program failed to load or link" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    checkGLError("Shader loading");

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
    std::cout << "Shader program linked successfully, ID: " << shader.ID << std::endl;

    std::string baldHeadPath = "models/bald_head.obj";
    if (!checkFileExists(baldHeadPath)) {
        std::cout << "Model file missing: " << baldHeadPath << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "Loading bald head model..." << std::endl;
    Model baldHead(baldHeadPath.c_str());
    std::cout << "Bald head loaded" << std::endl;

    if (!checkFileExists(currentHairPath)) {
        std::cout << "Initial hair model missing: " << currentHairPath << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    std::cout << "Loading initial hair model..." << std::endl;
    Model hair(currentHairPath.c_str());
    std::cout << "Initial hair model loaded" << std::endl;

    auto baldBox = baldHead.getBoundingBox();
    auto hairBox = hair.getBoundingBox();
    glm::vec3 baldSize = baldBox.max - baldBox.min;
    glm::vec3 hairSize = hairBox.max - hairBox.min;
    glm::vec3 baldCenter = (baldBox.min + baldBox.max) * 0.5f;
    glm::vec3 hairCenter = (hairBox.min + hairBox.max) * 0.5f;
    float targetScale = 1.0f;
    hairScaleValue = targetScale;
    hairPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    hairRotation = 0.0f;
    hairRotationX = 0.0f;
    hairRotationZ = 0.0f;

    std::cout << "Bald Box: min(" << baldBox.min.x << ", " << baldBox.min.y << ", " << baldBox.min.z << "), max("
        << baldBox.max.x << ", " << baldBox.max.y << ", " << baldBox.max.z << ")\n";
    std::cout << "Hair Box: min(" << hairBox.min.x << ", " << hairBox.min.y << ", " << hairBox.min.z << "), max("
        << hairBox.max.x << ", " << hairBox.max.y << ", " << hairBox.max.z << ")\n";
    std::cout << "Initial Hair Position: (" << hairPosition.x << ", " << hairPosition.y << ", " << hairPosition.z << ")\n";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    glm::vec3 lightPos(2.0f, 2.0f, 5.0f);
    glm::vec3 lightColor(1.5f, 1.5f, 1.5f);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, hair);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hair Adjustment");
        ImGui::Text("Hair Model Selection");
        if (ImGui::Button("Select Hair Model")) {
            IGFD::FileDialogConfig config;
            config.path = "models/";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseHairDlgKey", "Select Hair Model", ".obj,.ply", config);
        }
        ImGui::SameLine();
        ImGui::Text("Current: %s", currentHairPath.c_str());

        if (ImGuiFileDialog::Instance()->Display("ChooseHairDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                currentHairPath = ImGuiFileDialog::Instance()->GetFilePathName();
                if (checkFileExists(currentHairPath)) {
                    hair = Model(currentHairPath.c_str());
                    std::cout << "Loaded hair model: " << currentHairPath << std::endl;
                    hairPosition = glm::vec3(0.0f, 0.0f, 0.0f);
                    hairScaleValue = 1.0f;
                    hairRotation = 0.0f;
                    hairRotationX = 0.0f;
                    hairRotationZ = 0.0f;
                }
                else {
                    std::cout << "Failed to load hair model: " << currentHairPath << std::endl;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }
        ImGui::Text("Hair Color");
        ImGui::ColorEdit3("##HairColor", glm::value_ptr(hairColor));
        ImGui::Text("Hair Position");
        ImGui::Text("X");
        ImGui::SameLine();
        if (ImGui::Button("##PosXUp", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairPosition.x += adjustSpeed * deltaTime;
        }
        ImGui::SameLine();
        if (ImGui::Button("##PosXDown", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairPosition.x -= adjustSpeed * deltaTime;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("##PosX", &hairPosition.x, -10.0f, 10.0f);
        ImGui::Text("Y");
        ImGui::SameLine();
        if (ImGui::Button("##PosYUp", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairPosition.y += adjustSpeed * deltaTime;
        }
        ImGui::SameLine();
        if (ImGui::Button("##PosYDown", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairPosition.y -= adjustSpeed * deltaTime;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("##PosY", &hairPosition.y, -10.0f, 10.0f);
        ImGui::Text("Z");
        ImGui::SameLine();
        if (ImGui::Button("##PosZUp", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairPosition.z += adjustSpeed * deltaTime;
        }
        ImGui::SameLine();
        if (ImGui::Button("##PosZDown", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairPosition.z -= adjustSpeed * deltaTime;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("##PosZ", &hairPosition.z, -10.0f, 10.0f);

        ImGui::Text("Hair Scale");
        if (ImGui::Button("##ScaleUp", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairScaleValue += scaleSpeed * deltaTime;
        }
        ImGui::SameLine();
        if (ImGui::Button("##ScaleDown", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairScaleValue -= scaleSpeed * deltaTime;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("##Scale", &hairScaleValue, 0.1f, 20.0f);

        ImGui::Text("Hair Rotation");
        ImGui::Text("Y Rotation");
        ImGui::SameLine();
        if (ImGui::Button("##RotYUp", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairRotation += rotationSpeed * deltaTime;
        }
        ImGui::SameLine();
        if (ImGui::Button("##RotYDown", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairRotation -= rotationSpeed * deltaTime;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("##RotY", &hairRotation, -180.0f, 180.0f);
        ImGui::Text("X Rotation");
        ImGui::SameLine();
        if (ImGui::Button("##RotXUp", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairRotationX += rotationSpeed * deltaTime;
        }
        ImGui::SameLine();
        if (ImGui::Button("##RotXDown", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairRotationX -= rotationSpeed * deltaTime;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("##RotX", &hairRotationX, -180.0f, 180.0f);
        ImGui::Text("Z Rotation");
        ImGui::SameLine();
        if (ImGui::Button("##RotZUp", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairRotationZ += rotationSpeed * deltaTime;
        }
        ImGui::SameLine();
        if (ImGui::Button("##RotZDown", ImVec2(20, 20)) || (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            hairRotationZ -= rotationSpeed * deltaTime;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("##RotZ", &hairRotationZ, -180.0f, 180.0f);

        if (ImGui::Button("Reset to Auto Position")) {
            hairScaleValue = targetScale;
            hairPosition = glm::vec3(0.0f, 0.0f, 0.0f);
            hairRotation = 0.0f;
            hairRotationX = 0.0f;
            hairRotationZ = 0.0f;
        }

        if (ImGui::Button("Save Hair Model")) {
            showSaveConfirmation = true;
        }
        if (showSaveConfirmation) {
            ImGui::OpenPopup("Save Confirmation");
            if (ImGui::BeginPopupModal("Save Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Are you sure you want to overwrite the original hair file?\nThis action cannot be undone.");
                if (ImGui::Button("Yes", ImVec2(120, 0))) {
                    glm::mat4 hairModel = glm::mat4(1.0f);
                    hairModel = glm::translate(hairModel, hairPosition);
                    hairModel = glm::rotate(hairModel, glm::radians(hairRotation), glm::vec3(0.0f, 1.0f, 0.0f));
                    hairModel = glm::rotate(hairModel, glm::radians(hairRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
                    hairModel = glm::rotate(hairModel, glm::radians(hairRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
                    hairModel = glm::scale(hairModel, glm::vec3(hairScaleValue));
                    hair.saveToOBJ(currentHairPath, hairModel);
                    showSaveConfirmation = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(120, 0))) {
                    showSaveConfirmation = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
        ImGui::End();

        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(fov), 1280.0f / 720.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", cameraPos);
        shader.setVec3("lightColor", lightColor);

        if (renderBald) {
            glm::mat4 baldModel = glm::mat4(1.0f);
            baldModel = glm::translate(baldModel, glm::vec3(0.0f, 0.0f, 0.0f));
            baldModel = glm::scale(baldModel, glm::vec3(targetScale));
            shader.setMat4("model", baldModel);
            shader.setVec3("objectColor", glm::vec3(1.0f, 0.9f, 0.7f));
            baldHead.Draw(shader);
            checkGLError("Bald head render");
        }

        if (renderHair) {
            glm::mat4 hairModel = glm::mat4(1.0f);
            hairModel = glm::translate(hairModel, hairPosition);
            hairModel = glm::rotate(hairModel, glm::radians(hairRotation), glm::vec3(0.0f, 1.0f, 0.0f));
            hairModel = glm::rotate(hairModel, glm::radians(hairRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
            hairModel = glm::rotate(hairModel, glm::radians(hairRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
            hairModel = glm::scale(hairModel, glm::vec3(hairScaleValue));
            shader.setMat4("model", hairModel);
            shader.setVec3("objectColor", hairColor);
            hair.Draw(shader);
            checkGLError("Hair render");
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}