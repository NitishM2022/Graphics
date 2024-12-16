#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "common.h"

using namespace std;

class Sphere : public Shape 
{
public:
    Sphere(glm::vec3 position, float radius, Material color) : position(position), radius(radius), color(color) 
    {
    }

    ~Sphere() {}

    bool intersect(glm::vec3 origin, glm::vec3 ray, Hit& closestHit) override {
        glm::vec3 sphereToRayOrigin = origin - position;
        float a = dot(ray, ray);
        float b = 2.0f * dot(ray, sphereToRayOrigin);
        float c = dot(sphereToRayOrigin, sphereToRayOrigin) - radius * radius;
        float d = b * b - 4.0f * a * c;
                
        if (d < 0.0001f) {
            // No intersection
            return false;
        } else {
            float t1 = (-b - sqrt(d)) / (2.0f * a);
            float t2 = (-b + sqrt(d)) / (2.0f * a);

            Hit hit1;
            Hit hit2;

            if (t1 >= 0.0f) {
                glm::vec3 hitPos = origin + t1 * ray;
                glm::vec3 normal = glm::normalize((hitPos - position)/radius);
                hit1 = Hit(hitPos, normal, t1);
            }

            if (t2 >= 0.0f) {
                glm::vec3 hitPos = origin + t2 * ray;
                glm::vec3 normal = glm::normalize((hitPos - position)/radius);
                hit2 = Hit(hitPos, normal, t2);
            }

            // Why did I waste 5 hours on this??
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
    glm::vec3 position;
    float radius;
    Material color;
};


#endif
