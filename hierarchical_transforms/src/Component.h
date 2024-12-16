#ifndef COMPONENT_H
#define COMPONENT_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"

class Component{
public:
    glm::vec3 jointTranslation; // Translation of the component’s joint with respect to the parent component’s joint
    glm::vec3 jointAngles; // Current joint angles about the X, Y, and Z axes of the component’s joint
    glm::vec3 meshTranslation; // Translation of the component’s mesh with respect to its joint
    glm::vec3 meshScale; // X, Y, and Z scaling factors for the mesh

    std::shared_ptr<Shape> cube;
    std::shared_ptr<Shape> sphere;
    std::shared_ptr<Program> prog;
    std::vector<std::shared_ptr<Component>> children;
    std::shared_ptr<Component> parent;
    bool rotate;
    int currentChildIndex = 0;

    Component(std::shared_ptr<Shape> c, std::shared_ptr<Shape> s, std::shared_ptr<Program> pr, std::shared_ptr<Component> parent = nullptr, bool rotate = false);

    void setJointTranslation(const glm::vec3& translation);
    void setJointAngles(const glm::vec3& angles);
    void setMeshTranslation(const glm::vec3& translation);
    void setMeshScale(const glm::vec3& scale);

    void draw(const std::shared_ptr<MatrixStack>& matrixStack, const std::shared_ptr<MatrixStack>& P);

    void addChild(std::shared_ptr<Component> child);
    std::shared_ptr<Component> getNextChild();
    std::shared_ptr<Component> getParent();
};

#endif 