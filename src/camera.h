#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class Camera {
public:
    // Enumeration for camera movement directions
    enum CameraMovement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

private:
    // Camera attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 worldUp;

    // Euler angles
    float yaw;
    float pitch;

    // Camera settings
    float movementSpeed;
    float mouseSensitivity;
    float fov;

public:
    // Constructor with default values
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.5f, 5.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f,
        float pitch = 0.0f)
        : front(glm::vec3(0.0f, 0.0f, -1.0f)),
        movementSpeed(2.5f),
        mouseSensitivity(0.1f),
        fov(60.0f),
        position(position),
        worldUp(up),
        yaw(yaw),
        pitch(pitch)
    {
        updateCameraVectors();
    }

    // Update yaw and pitch based on mouse movement
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // Limit the pitch to prevent flipping
        if (constrainPitch) {
            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
        }

        updateCameraVectors();
    }

    // Update camera position based on keyboard input
    void processKeyboard(CameraMovement direction, float deltaTime) {
        float velocity = movementSpeed * deltaTime;
        glm::vec3 right = glm::normalize(glm::cross(front, worldUp));

        if (direction == FORWARD)
            position += front * velocity;
        if (direction == BACKWARD)
            position -= front * velocity;
        if (direction == LEFT)
            position -= right * velocity;
        if (direction == RIGHT)
            position += right * velocity;
    }

    // Adjust field of view (zoom) using scroll input
    void processMouseScroll(float yoffset) {
        fov -= yoffset;
        if (fov < 30.0f) fov = 30.0f;
        if (fov > 90.0f) fov = 90.0f;
    }

    // Return view matrix for rendering
    glm::mat4 getViewMatrix() const {
        glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
        glm::vec3 cameraUp = glm::normalize(glm::cross(right, front));
        return glm::lookAt(position, position + front, cameraUp);
    }

    // Getters
    float getFov() const { return fov; }
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    float getMouseSensitivity() const { return mouseSensitivity; }

    // Setter
    void setMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }

private:
    // Recalculate front vector from updated yaw and pitch
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);
    }
};

#endif
