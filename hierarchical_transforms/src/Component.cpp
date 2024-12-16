#include "Component.h"

Component::Component(std::shared_ptr<Shape> c, std::shared_ptr<Shape> s, std::shared_ptr<Program> pr, std::shared_ptr<Component> parent, bool rotate)
    : cube(c), sphere(s), parent(parent), prog(pr), rotate(rotate) {
}

void Component::setJointTranslation(const glm::vec3& translation) {
    jointTranslation = translation;
}

void Component::setJointAngles(const glm::vec3& angles) {
    jointAngles = angles;
}

void Component::setMeshTranslation(const glm::vec3& translation) {
    meshTranslation = translation;
}

void Component::setMeshScale(const glm::vec3& scale) {
    meshScale = scale;
}

void Component::draw(const std::shared_ptr<MatrixStack>& matrixStack, const std::shared_ptr<MatrixStack>& P) {
    matrixStack->pushMatrix();
        matrixStack->translate(jointTranslation);
        if (parent) {
            matrixStack->pushMatrix();
                matrixStack->scale(0.5); // Adjust the scale as needed
                glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
                glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &matrixStack->topMatrix()[0][0]);
                sphere->draw(prog);
            matrixStack->popMatrix();
        }
        matrixStack->rotate(jointAngles.x, glm::vec3(1, 0, 0));
        matrixStack->rotate(jointAngles.y, glm::vec3(0, 1, 0));
        matrixStack->rotate(jointAngles.z, glm::vec3(0, 0, 1));

        matrixStack->pushMatrix();
            matrixStack->translate(meshTranslation);
            matrixStack->scale(meshScale);
            if(rotate){
                matrixStack->rotate((float) glfwGetTime() * 3, glm::vec3(1, 0, 0));
            }
            glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
            glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &matrixStack->topMatrix()[0][0]);
            cube->draw(prog);
        matrixStack->popMatrix();

        for (auto& child : children) {
            child->draw(matrixStack, P);
        }
    matrixStack->popMatrix();
}

void Component::addChild(std::shared_ptr<Component> child) {
    children.push_back(child);
}

std::shared_ptr<Component> Component::getNextChild() {
    if (children.empty()) {
        return nullptr;
    }
    auto nextChild = children[currentChildIndex];
    currentChildIndex = (currentChildIndex + 1) % children.size();
    return nextChild;
}

std::shared_ptr<Component> Component::getParent() {
    return parent;
}