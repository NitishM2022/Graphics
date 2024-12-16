#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "Program.h"

using namespace std;

class Light {
public:
    int id;
    glm::vec3 position;
    glm::vec3 color;

    Light(int id, glm::vec3 position, glm::vec3 color)
        : id(id), position(position), color(color) {}

    void setUniforms(std::shared_ptr<Program> prog) {
        glUniform3fv(prog->getUniform(("lightPos"+to_string(id)).c_str()), 1, glm::value_ptr(position));
        glUniform3fv(prog->getUniform(("lightCol"+to_string(id)).c_str()), 1, glm::value_ptr(color));
    }
};

#endif