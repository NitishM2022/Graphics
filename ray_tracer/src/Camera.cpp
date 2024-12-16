#include "Camera.h"

#include <iostream>
#include <glm/gtc/constants.hpp>
#include <cmath>

using namespace std;

Camera::Camera(int width, int height, float fov, float aspect, glm::vec3 position, glm::vec3 front, glm::vec3 up)
    : width(width), height(height), fov(glm::radians(fov)), aspect(aspect), position(position), front(glm::normalize(front)), up(glm::normalize(up))
{
}

vector<glm::vec3> Camera::genRays(vector<glm::vec3>& rays)
{
    glm::vec3 right = glm::normalize(glm::cross(up, front));
    up = glm::normalize(glm::cross(right, front));

    float tanHalfFOV = tan(fov / 2.0f);
    
    /*                (Gives Dx since adj * Opp/adj) (scale with aspect) (Gets full width since have of FOV obly gets top half)  */
    float fullWidth = tanHalfFOV * aspect * 2;
    float fullHeight = tanHalfFOV * 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float dx = (fullWidth  *  (x * 2 + 1) / (width * 2) - fullWidth / 2);
            float dy = (fullHeight *  (y * 2 + 1) / (height * 2) - fullHeight / 2);

            glm::vec3 planeIntersection = dx * right + dy * up + front;
            glm::vec3 rayDirection = glm::normalize(planeIntersection);
            
            rays.push_back(rayDirection);
        }
    }

    return rays;
}

void Camera::applyViewMatrix(shared_ptr<MatrixStack> MV)
{
    MV->translate(-position);
    // glm::vec3 right = glm::normalize(glm::cross(front, up));
    // up = glm::normalize(glm::cross(right, front));
    // MV->multMatrix(glm::lookAt(position, position + front, up));
}
