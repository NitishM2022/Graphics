#ifndef COMMON_H
#define COMMON_H

#include <glm/glm.hpp>
#include <vector>

class Hit
{
public:
	Hit() : x(0), n(0), t(0), valid(false) {}
	Hit(const glm::vec3 &x, const glm::vec3 &n, float t) { this->x = x; this->n = n; this->t = t; this->valid = true;}
	glm::vec3 x; // position
	glm::vec3 n; // normal
	float t; // distance
    bool valid;
};

struct Material
{
    glm::vec3 diff;
    glm::vec3 spec;
    glm::vec3 amb;
    float exp;
    bool isReflective = false;
};

struct Light {
    glm::vec3 position;
    float intensity;
};

class Shape {
public:
    virtual bool intersect(glm::vec3 origin, glm::vec3 ray, Hit& hit) = 0; // Pure virtual function
    virtual Material getColor() = 0;
};

class Scene {
public:


    void addShape(Shape* shape) {
        shapes.push_back(shape);
    }

    std::vector<Shape*>& getShapes() {
        return shapes;
    }

    bool hit(const glm::vec3 &origin, const glm::vec3 &ray, Hit &closestHit, Material &closestMaterial) {
        bool atleastOneHit = false;
        
        for(Shape* shape : shapes){
            Hit closestShapeHit;
            bool rayHit = shape->intersect(origin, ray, closestShapeHit);
            if(closestHit.valid == false && rayHit == true){
                closestHit.valid = true; // this is lowkey redundant

                closestHit = closestShapeHit;
                closestMaterial = shape->getColor();

                atleastOneHit = true;
            } else if(closestHit.valid == true && rayHit == true && closestHit.t > closestShapeHit.t){
                closestHit = closestShapeHit;
                closestMaterial = shape->getColor();

                atleastOneHit = true;
            }
        }
        return atleastOneHit;
    }


private:
    std::vector<Shape*> shapes;
};

#endif