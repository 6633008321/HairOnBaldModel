#ifndef UI_H
#define UI_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "ImGuiFileDialog.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include "model.h"
#include "hair_transform.h"

// Class to manage the ImGui user interface for hair model adjustments
class UI {
private:
    bool showSaveConfirmation;    // Flag for showing save confirmation popup
    bool* wireframeMode;          // Pointer to wireframe mode toggle
    bool* renderBald;             // Pointer to bald rendering toggle
    bool* renderHair;             // Pointer to hair rendering toggle
    bool* mouseLocked;            // Pointer to mouse lock state
    HairTransform* hairTransform; // Pointer to hair transformation data
    Model* hairModel;             // Pointer to hair model

public:
    // Constructor initializes UI with references to external states
    UI(bool* wireframeMode, bool* renderBald, bool* renderHair, bool* mouseLocked,
        HairTransform* hairTransform, Model* hairModel)
        : showSaveConfirmation(false),
        wireframeMode(wireframeMode),
        renderBald(renderBald),
        renderHair(renderHair),
        mouseLocked(mouseLocked),
        hairTransform(hairTransform),
        hairModel(hairModel) {
    }

    // Initializes ImGui context and backends
    void initialize(GLFWwindow* window) {
        // Check ImGui version and create context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Configure ImGui IO settings
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard; // Disable keyboard navigation

        // Set dark theme and initialize backends
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
    }

    // Prepares a new ImGui frame
    void newFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    // Renders the main UI window with hair adjustment controls
    void renderUI(float deltaTime) {
        ImGui::Begin("Hair Adjustment");

        // Debug mouse and window state when mouse is unlocked
        if (!*mouseLocked) {
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
        }

        // Additional debug info when mouse is unlocked
        ImGuiIO& io = ImGui::GetIO();
        if (!*mouseLocked) {
            bool isWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow |
                ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
            bool isWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);        
        }

        // Hair model selection section
        ImGui::Text("Hair Model Selection");
        if (ImGui::Button("Select Hair Model")) {
            IGFD::FileDialogConfig config;
            config.path = "models/";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseHairDlgKey", "Select Hair Model", ".obj,.ply", config);
        }

        ImGui::SameLine();
        ImGui::Text("Current: %s", hairTransform->getModelPath().c_str());

        // Handle file dialog for model selection
        handleFileDialog();

        // Hair color adjustment
        ImGui::Text("Hair Color");
        glm::vec3 color = hairTransform->getColor();
        if (ImGui::ColorEdit3("##HairColor", glm::value_ptr(color))) {
            hairTransform->setColor(color);
        }

        // Position, scale, and rotation controls
        renderPositionControls(deltaTime);
        renderScaleControls(deltaTime);
        renderRotationControls(deltaTime);

        // Reset transformation button
        if (ImGui::Button("Reset to Auto Position")) {
            hairTransform->reset(1.0f);
        }

        // Save model button
        if (ImGui::Button("Save Hair Model")) {
            showSaveConfirmation = true;
        }

        // Handle save confirmation popup
        handleSaveConfirmation();

        ImGui::End();
    }

    // Renders ImGui draw data
    void renderEndFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Cleans up ImGui resources
    void cleanup() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

private:
    // Handles file dialog for selecting hair model
    void handleFileDialog() {
        if (ImGuiFileDialog::Instance()->Display("ChooseHairDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
                std::ifstream file(path);
                if (file.good()) {
                    hairTransform->setModelPath(path);
                    *hairModel = Model(path.c_str());
                    std::cout << "Loaded hair model: " << path << std::endl;
                    hairTransform->reset(1.0f);
                }
                else {
                    std::cout << "Failed to load hair model: " << path << std::endl;
                }
                file.close();
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }

    // Renders position adjustment controls
    void renderPositionControls(float deltaTime) {
        ImGui::Text("Hair Position");
        glm::vec3 position = hairTransform->getPosition();
        bool positionChanged = false;

        // X position controls
        ImGui::Text("X");
        ImGui::SameLine();
        if (ImGui::Button("##PosXUp", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            position.x += hairTransform->getAdjustSpeed() * deltaTime;
            positionChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("##PosXDown", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            position.x -= hairTransform->getAdjustSpeed() * deltaTime;
            positionChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat("##PosX", &position.x, -10.0f, 10.0f)) {
            positionChanged = true;
        }

        // Y position controls
        ImGui::Text("Y");
        ImGui::SameLine();
        if (ImGui::Button("##PosYUp", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            position.y += hairTransform->getAdjustSpeed() * deltaTime;
            positionChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("##PosYDown", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            position.y -= hairTransform->getAdjustSpeed() * deltaTime;
            positionChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat("##PosY", &position.y, -15.0f, 15.0f)) {
            positionChanged = true;
        }

        // Z position controls
        ImGui::Text("Z");
        ImGui::SameLine();
        if (ImGui::Button("##PosZUp", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            position.z += hairTransform->getAdjustSpeed() * deltaTime;
            positionChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("##PosZDown", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            position.z -= hairTransform->getAdjustSpeed() * deltaTime;
            positionChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat("##PosZ", &position.z, -10.0f, 10.0f)) {
            positionChanged = true;
        }

        // Apply position changes
        if (positionChanged) {
            hairTransform->setPosition(position);
        }
    }

    // Renders scale adjustment controls
    void renderScaleControls(float deltaTime) {
        ImGui::Text("Hair Scale");
        float scale = hairTransform->getScale();
        bool scaleChanged = false;

        if (ImGui::Button("##ScaleUp", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            scale += hairTransform->getScaleSpeed() * deltaTime;
            scaleChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("##ScaleDown", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            scale -= hairTransform->getScaleSpeed() * deltaTime;
            scaleChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat("##Scale", &scale, 0.1f, 20.0f)) {
            scaleChanged = true;
        }

        // Apply scale changes
        if (scaleChanged) {
            hairTransform->setScale(scale);
        }
    }

    // Renders rotation adjustment controls
    void renderRotationControls(float deltaTime) {
        ImGui::Text("Hair Rotation");
        float rotY = hairTransform->getRotationY();
        float rotX = hairTransform->getRotationX();
        float rotZ = hairTransform->getRotationZ();
        bool rotationChanged = false;

        // Y rotation controls
        ImGui::Text("Y Rotation");
        ImGui::SameLine();
        if (ImGui::Button("##RotYUp", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            rotY += hairTransform->getRotationSpeed() * deltaTime;
            rotationChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("##RotYDown", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            rotY -= hairTransform->getRotationSpeed() * deltaTime;
            rotationChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat("##RotY", &rotY, -180.0f, 180.0f)) {
            rotationChanged = true;
        }

        // X rotation controls
        ImGui::Text("X Rotation");
        ImGui::SameLine();
        if (ImGui::Button("##RotXUp", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            rotX += hairTransform->getRotationSpeed() * deltaTime;
            rotationChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("##RotXDown", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            rotX -= hairTransform->getRotationSpeed() * deltaTime;
            rotationChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat("##RotX", &rotX, -180.0f, 180.0f)) {
            rotationChanged = true;
        }

        // Z rotation controls
        ImGui::Text("Z Rotation");
        ImGui::SameLine();
        if (ImGui::Button("##RotZUp", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            rotZ += hairTransform->getRotationSpeed() * deltaTime;
            rotationChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("##RotZDown", ImVec2(20, 20)) ||
            (ImGui::IsItemActive() && ImGui::IsMouseDown(0))) {
            rotZ -= hairTransform->getRotationSpeed() * deltaTime;
            rotationChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat("##RotZ", &rotZ, -180.0f, 180.0f)) {
            rotationChanged = true;
        }

        // Apply rotation changes
        if (rotationChanged) {
            hairTransform->setRotation(rotY, rotX, rotZ);
        }
    }

    // Handles save confirmation popup
    void handleSaveConfirmation() {
        if (showSaveConfirmation) {
            ImGui::OpenPopup("Save Confirmation");
            if (ImGui::BeginPopupModal("Save Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Are you sure you want to overwrite the original hair file?\nThis action cannot be undone.");

                if (ImGui::Button("Yes", ImVec2(120, 0))) {
                    glm::mat4 transformMatrix = hairTransform->getModelMatrix();
                    hairModel->saveToOBJ(hairTransform->getModelPath(), transformMatrix);
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
    }
};

#endif