#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "common.h"

using namespace std;

class Plane : public Shape 
{
public:
    Plane(glm::vec3 position, glm::vec3 normal, Material color) : position(position), normal(normal), color(color) 
    {
    }

    ~Plane() {}

    bool intersect(glm::vec3 origin, glm::vec3 ray, Hit& closestHit) override {
        glm::vec3 planeToRayOrigin = position - origin;
        float t = dot(normal, planeToRayOrigin) / dot(normal, ray);

        if(t < 0.0001f){
            return false;
        }

        glm::vec3 x = origin + t * ray;

        Hit hit(x, normal, t);

        closestHit = hit;
        
        return true;
    }

    Material getColor() override { return color; }

private:
    glm::vec3 position;
    glm::vec3 normal;
    Material color;
};


#endif
