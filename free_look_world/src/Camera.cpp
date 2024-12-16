#define _USE_MATH_DEFINES
#include <cmath> 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "MatrixStack.h"

Camera::Camera() :
    position(0.0f, 0.0f, 0.0f), // Set initial camera position (height = 1.0)
	aspect(1.0f),
    fovy(45.0f * M_PI / 180.0f),
    znear(0.1f),
    zfar(1000.0f),
	yaw(0.0f),
    pitch(0.0f),
    mouseSensitivity(0.005f),
    tfactor(1.0f)
{
    updateCameraVectors();
}

Camera::~Camera()
{
}

void Camera::setInitDistance(float z) {
	position.z = position.z - std::abs(z);
}

void Camera::setInitHeight(float y) {
	position.y = position.y + std::abs(y);
}

void Camera::updateCameraVectors() {
    glm::vec3 direction;
    direction.x = sin(yaw);
    direction.y = sin(pitch);
    direction.z = cos(yaw);
    front = glm::normalize(direction);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::setAspect(float a) {
    aspect = a;
}

void Camera::moveForward(float dt) {
	glm::vec3 tempFront = glm::normalize(glm::cross(worldUp, right));
    position += tempFront * tfactor * dt;
}

void Camera::moveBackward(float dt) {
	glm::vec3 tempFront = glm::normalize(glm::cross(worldUp, right));
    position -= tempFront * tfactor * dt;
}

void Camera::moveLeft(float dt) {
    position -= right * tfactor * dt;
}

void Camera::moveRight(float dt) {
    position += right * tfactor * dt;
}

void Camera::mouseMoved(float xpos, float ypos) {
    float xoffset = xpos * mouseSensitivity;
    float yoffset = ypos * mouseSensitivity;

    yaw -= xoffset;
    pitch += yoffset;

    pitch = glm::clamp(pitch, static_cast<float>(-M_PI / 3.0f + 0.1f), static_cast<float>(M_PI / 3.0f - 0.1f));

    updateCameraVectors();
}

void Camera::zoomIn(float dt) {
    fovy = glm::clamp(fovy - dt, static_cast<float>(4.0f * M_PI / 180.0f), static_cast<float>(114.0f * M_PI / 180.0f));
}

void Camera::zoomOut(float dt) {
    fovy = glm::clamp(fovy + dt, static_cast<float>(4.0f * M_PI / 180.0f), static_cast<float>(114.0f * M_PI / 180.0f));
}

void Camera::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const {
    P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
}


void Camera::applyViewMatrix(std::shared_ptr<MatrixStack> MV) const {
    MV->multMatrix(glm::lookAt(position, position + front, up));
}

glm::mat4 Camera::getCameraMatrix() {
	return glm::inverse(glm::lookAt(position, position + front, up));
}

