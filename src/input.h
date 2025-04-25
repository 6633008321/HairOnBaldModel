#ifndef INPUT_H
#define INPUT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "camera.h"  
#include "hair_transform.h" 
#include "model.h"     
#include "ImGuiFileDialog.h"
#include <imgui.h>

class InputManager {

private:
    GLFWwindow* window;    
    Camera* camera;             // Camera object for controlling the camera
    HairTransform* hairTransform; // Hair transformation object for modifying hair position/rotation
    bool* wireframeMode;        // Pointer to wireframe mode toggle
    bool* renderBald;           // Pointer to render bald mode toggle
    bool* renderHair;           // Pointer to render hair mode toggle
    bool* mouseLocked;          // Pointer to mouse lock status
    float lastKeyPressTime;     // Time tracking to prevent key spamming
    const float keyCooldown = 0.2f;  // Key cooldown time to prevent accidental multiple key presses

public:
    // Constructor to initialize InputManager with window, camera, and other required parameters
    InputManager(GLFWwindow* window, Camera* camera, HairTransform* hairTransform,
        bool* wireframeMode, bool* renderBald, bool* renderHair, bool* mouseLocked) :
        window(window),
        camera(camera),
        hairTransform(hairTransform),
        wireframeMode(wireframeMode),
        renderBald(renderBald),
        renderHair(renderHair),
        mouseLocked(mouseLocked),
        lastKeyPressTime(0.0f) {
    }

    // Set up GLFW callbacks for input handling
    void setupCallbacks() {
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // Resizes the framebuffer
        glfwSetWindowUserPointer(window, this);  // Store pointer to this instance for static callbacks
    }

    // Input processing in the main loop
    void processInput(float deltaTime) {
        float currentTime = static_cast<float>(glfwGetTime()); // Get the current time to track key press timings

        // Check if the escape key is pressed to close the window
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Camera movement using WASD keys (ignores mouse lock status)
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->processKeyboard(Camera::CameraMovement::FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->processKeyboard(Camera::CameraMovement::BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->processKeyboard(Camera::CameraMovement::LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->processKeyboard(Camera::CameraMovement::RIGHT, deltaTime);

        // Toggle wireframe mode using the 'F' key
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
            *wireframeMode = !(*wireframeMode);
            glPolygonMode(GL_FRONT_AND_BACK, *wireframeMode ? GL_LINE : GL_FILL); // Toggle between wireframe and fill mode
            lastKeyPressTime = currentTime;
        }

        // Toggle rendering modes (bald, hair, both) with keys 1, 2, and 3
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
            *renderBald = true;
            *renderHair = false;
            std::cout << "Rendering bald head only" << std::endl;
            lastKeyPressTime = currentTime;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
            *renderBald = false;
            *renderHair = true;
            std::cout << "Rendering hair only" << std::endl;
            lastKeyPressTime = currentTime;
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
            *renderBald = true;
            *renderHair = true;
            std::cout << "Rendering both" << std::endl;
            lastKeyPressTime = currentTime;
        }

        // Toggle mouse lock using the 'TAB' key
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
            *mouseLocked = !(*mouseLocked);
            glfwSetInputMode(window, GLFW_CURSOR, *mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL); // Toggle cursor mode
            lastKeyPressTime = currentTime;
            std::cout << "Mouse " << (*mouseLocked ? "locked" : "unlocked") << ", Cursor mode: "
                << (*mouseLocked ? "DISABLED" : "NORMAL") << std::endl;

            // Debug cursor position after toggle
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
        }

        // Open file dialog with the 'O' key
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && currentTime - lastKeyPressTime > keyCooldown) {
            ImGuiIO& io = ImGui::GetIO();
            if (!io.WantCaptureKeyboard) {
                IGFD::FileDialogConfig config;
                config.path = "models/";
                ImGuiFileDialog::Instance()->OpenDialog("ChooseHairDlgKey", "Select Hair Model", ".obj,.ply", config);
                lastKeyPressTime = currentTime;
            }
        }

        // Adjust hair position using I, J, K, L keys (works regardless of mouse lock or ImGui focus)
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            hairTransform->adjustPosition(0.0f, 1.0f, 0.0f, deltaTime); // Move up
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            hairTransform->adjustPosition(0.0f, -1.0f, 0.0f, deltaTime); // Move down
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            hairTransform->adjustPosition(-1.0f, 0.0f, 0.0f, deltaTime); // Move left
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
            hairTransform->adjustPosition(1.0f, 0.0f, 0.0f, deltaTime); // Move right

        // Adjust hair rotation using Q, E keys (rotate around Y-axis)
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            hairTransform->adjustRotation(0.0f, 1.0f, 0.0f, deltaTime); // Rotate Y+
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            hairTransform->adjustRotation(0.0f, -1.0f, 0.0f, deltaTime); // Rotate Y-
    }

private:
    // --- Static Callbacks ---

    // Callback to resize the framebuffer when the window is resized
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    // Callback for mouse movement (modified)
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (!inputManager) return; // Safety check in case of errors

        ImGuiIO& io = ImGui::GetIO();

        // If ImGui wants to capture mouse input, or if the mouse is unlocked, do not process camera movement
        if (io.WantCaptureMouse || !*(inputManager->mouseLocked)) {
            return;
        }

        // If mouse is locked and ImGui is not capturing the mouse, process camera movement
        inputManager->camera->processMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    // Callback for mouse scroll (modified)
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (!inputManager) return; // Safety check

        ImGuiIO& io = ImGui::GetIO();

        // If ImGui wants to capture mouse input, ignore camera scroll input
        if (io.WantCaptureMouse) {
            return;
        }

        // Otherwise, process the scroll for the camera (e.g., zoom in/out)
        if (inputManager) {
            inputManager->camera->processMouseScroll(static_cast<float>(yoffset));
        }
    }

};

#endif // INPUT_H
