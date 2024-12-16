#pragma  once
#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class MatrixStack;

class Camera {
public:
	enum {
		ROTATE = 0,
		TRANSLATE,
		SCALE
	};

    Camera();
	~Camera();
	void setInitDistance(float z);
	void setInitHeight(float y);
    void setAspect(float a);
    void moveForward(float dt);
    void moveBackward(float dt);
    void moveLeft(float dt);
    void moveRight(float dt);
    void mouseMoved(float xpos, float ypos);
    void zoomIn(float dt);
    void zoomOut(float dt);
    void applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const;
    void applyViewMatrix(std::shared_ptr<MatrixStack> MV) const;
	glm::vec3 getCameraPosition() { return position; }
	glm::mat4 getCameraMatrix();
	float getFovy() { return fovy; }
    glm::vec3 getPosition() { return position; }

private:
	float aspect;
	float fovy;
    float znear;
    float zfar;
    float yaw;
    float pitch;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float tfactor;

    float mouseSensitivity;
	void updateCameraVectors();
};

#endif
