#ifndef HAIR_TRANSFORM_H
#define HAIR_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class HairTransform {
private:
    // Transformation properties
    glm::vec3 position;
    float scaleValue;
    float rotationY;
    float rotationX;
    float rotationZ;

    // Appearance
    glm::vec3 color;
    std::string modelPath;

    // Adjustment speeds
    const float adjustSpeed = 0.5f;
    const float scaleSpeed = 0.05f;
    const float rotationSpeed = 5.0f;

public:
    // Default constructor
    HairTransform()
        : position(0.0f),
        scaleValue(0.5f),
        rotationY(0.0f),
        rotationX(0.0f),
        rotationZ(0.0f),
        color(0.5f, 0.3f, 0.2f),
        modelPath("models/hair_front.obj") {
    }

    // Reset all transformations to default
    void reset(float targetScale = 0.5f) {
        position = glm::vec3(0.0f);
        scaleValue = targetScale;
        rotationY = 0.0f;
        rotationX = 0.0f;
        rotationZ = 0.0f;
    }

    // Position adjustment
    void adjustPosition(float x, float y, float z, float deltaTime) {
        position.x += x * adjustSpeed * deltaTime;
        position.y += y * adjustSpeed * deltaTime;
        position.z += z * adjustSpeed * deltaTime;
    }

    // Scale adjustment
    void adjustScale(float amount, float deltaTime) {
        scaleValue += amount * scaleSpeed * deltaTime;
        if (scaleValue < 0.1f) scaleValue = 0.1f;
    }

    // Rotation adjustment (Yaw, Pitch, Roll)
    void adjustRotation(float yaw, float pitch, float roll, float deltaTime) {
        rotationY += yaw * rotationSpeed * deltaTime;
        rotationX += pitch * rotationSpeed * deltaTime;
        rotationZ += roll * rotationSpeed * deltaTime;
    }

    // Setters
    void setPosition(const glm::vec3& newPos) { position = newPos; }
    void setScale(float scale) { scaleValue = scale; }
    void setRotation(float yaw, float pitch, float roll) {
        rotationY = yaw;
        rotationX = pitch;
        rotationZ = roll;
    }
    void setColor(const glm::vec3& newColor) { color = newColor; }
    void setModelPath(const std::string& path) { modelPath = path; }

    // Getters
    glm::vec3 getPosition() const { return position; }
    float getScale() const { return scaleValue; }
    float getRotationY() const { return rotationY; }
    float getRotationX() const { return rotationX; }
    float getRotationZ() const { return rotationZ; }
    glm::vec3 getColor() const { return color; }
    std::string getModelPath() const { return modelPath; }

    float getAdjustSpeed() const { return adjustSpeed; }
    float getScaleSpeed() const { return scaleSpeed; }
    float getRotationSpeed() const { return rotationSpeed; }

    // Get model matrix combining all transformations
    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(scaleValue));
        return model;
    }
};

#endif
