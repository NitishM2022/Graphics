#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "MatrixStack.h"
#include "common.h"

using namespace std;

class Ellipsoid : public Shape 
{
public:
    Ellipsoid(glm::mat4 modelMatrix, Material color) : modelMatrix(modelMatrix), invModelMatrix(glm::inverse(modelMatrix)), color(color) 
    {
    }

    ~Ellipsoid() {}

    bool intersect(glm::vec3 origin, glm::vec3 ray, Hit& closestHit) override {
        glm::vec3 rayOriginToEllipsoidSpace = glm::vec3(invModelMatrix * glm::vec4(origin, 1.0f));
        glm::vec3 rayDirToEllipsoidSpace = glm::normalize(glm::vec3(invModelMatrix * glm::vec4(ray, 0.0f)));

        float a = dot(rayDirToEllipsoidSpace, rayDirToEllipsoidSpace);
        float b = 2.0f * dot(rayDirToEllipsoidSpace, rayOriginToEllipsoidSpace);
        float c = dot(rayOriginToEllipsoidSpace, rayOriginToEllipsoidSpace) - 1.0f;
        float d = b * b - 4.0f * a * c;

        if (d < 0.0001f) {
            return false;
        } else {
            float t1 = (-b - sqrt(d)) / (2.0f * a);
            glm::vec3 x1 = rayOriginToEllipsoidSpace + t1 * rayDirToEllipsoidSpace;

            float t2 = (-b + sqrt(d)) / (2.0f * a);
            glm::vec3 x2 = rayOriginToEllipsoidSpace + t2 * rayDirToEllipsoidSpace;

            Hit hit1;
            Hit hit2;

            if (t1 >= 0.0f) {
                glm::vec3 x = glm::vec3(modelMatrix * glm::vec4(x1, 1.0f));
                
                glm::vec3 normal = glm::normalize(glm::vec3(glm::transpose(invModelMatrix) * glm::vec4(x1, 0.0f)));

                float distance = glm::length(x - origin);
                hit1 = Hit(x, normal, distance);

                if(dot(ray, (x - origin)) < 0.0f){
                    hit1.valid = false;
                    hit1.t = -1 * hit2.t;
                }
            }

            if (t2 >= 0.0f) {
                glm::vec3 x = glm::vec3(modelMatrix * glm::vec4(x2, 1.0f));
                
                glm::vec3 normal = glm::normalize(glm::vec3(glm::transpose(invModelMatrix) * glm::vec4(x2, 0.0f)));

                float distance = glm::length(x - origin);
                hit2 = Hit(x, normal, distance);

                if(dot(ray, (x - origin)) < 0.0f){
                    hit2.valid = false;
                    hit2.t = -1 * hit2.t;
                }
            }

            if(!hit1.valid && !hit2.valid){
                return false;
            }

            Hit smallerHit;
            if(hit1.valid){
                smallerHit = hit1;
                if(hit2.valid && hit2.t < hit1.t){
                    smallerHit = hit2;
                }
            }else{
                smallerHit = hit2;
            }

            closestHit = smallerHit;
            
            return true;
        }
    }

    Material getColor() override { return color; }

private:
    shared_ptr<MatrixStack> M;
    glm::mat4 modelMatrix;
    glm::mat4 invModelMatrix;
    Material color;
};

#endif // ELLIPSOID_H