#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include <glm/glm.hpp>

#include "MatrixStack.h"

class Camera {
public:
    Camera(int width, int height, float fov, float aspect, glm::vec3 position, glm::vec3 front, glm::vec3 up);

    std::vector<glm::vec3> genRays(std::vector<glm::vec3>& rays);
    void applyViewMatrix(std::shared_ptr<MatrixStack> MV);


private:
    int width;
    int height;
    float fov;
    float aspect;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
};

#endif 