#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "Program.h"

using namespace std;

class Material {
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess)
        : ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {}

    void setUniforms(shared_ptr<Program> prog) {
		glUniform3fv(prog->getUniform("amb"), 1, glm::value_ptr(ambient));
		glUniform3fv(prog->getUniform("diff"), 1,  glm::value_ptr(diffuse));
		glUniform3fv(prog->getUniform("spec"), 1, glm::value_ptr(specular));
		glUniform1f(prog->getUniform("shiny"),  shininess);
    }
};

#endif 