#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Component.h"

#define M_PI 3.14159265358979323846

using namespace std;

GLFWwindow *window; // Main application window
string RES_DIR = ""; // Where data files live
shared_ptr<Program> prog;
shared_ptr<Program> progIM; // immediate mode
shared_ptr<Shape> cube;
shared_ptr<Shape> sphere;
shared_ptr<Component> torso;
std::shared_ptr<Component> currentComponent;


static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void init()
{
	GLSL::checkVersion();

	// Check how many texture units are supported in the vertex shader
	int tmp;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &tmp);
	cout << "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = " << tmp << endl;
	// Check how many uniforms are supported in the vertex shader
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &tmp);
	cout << "GL_MAX_VERTEX_UNIFORM_COMPONENTS = " << tmp << endl;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &tmp);
	cout << "GL_MAX_VERTEX_ATTRIBS = " << tmp << endl;

	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Initialize mesh.
	cube = make_shared<Shape>();
	cube->loadMesh(RES_DIR + "cube.obj");
	cube->init();

	sphere = make_shared<Shape>();
	sphere->loadMesh(RES_DIR + "sphere.obj");
	sphere->init();
	
	// Initialize the GLSL programs.
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RES_DIR + "vert.glsl", RES_DIR + "frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->setVerbose(false);
	
	progIM = make_shared<Program>();
	progIM->setVerbose(true);
	progIM->setShaderNames(RES_DIR + "vert.glsl", RES_DIR + "frag.glsl");
	progIM->init();
	progIM->addUniform("P");
	progIM->addUniform("MV");
	progIM->setVerbose(false);

    torso = std::make_shared<Component>(cube, sphere, prog);
    torso->setMeshTranslation(glm::vec3(0.0f, 0.0f, 0.0f));
    torso->setMeshScale(glm::vec3(1.0f, 1.5f, 0.6f));

    auto head = std::make_shared<Component>(cube, sphere, prog, torso);
    head->setJointTranslation(glm::vec3(0.0f, 0.75f, 0.0f));
	head->setMeshTranslation(glm::vec3(0.0f, 0.2f, 0.0f));
    head->setMeshScale(glm::vec3(0.4f, 0.4f, 0.4f));

    auto leftArm = std::make_shared<Component>(cube, sphere, prog, torso, true);
    leftArm->setJointTranslation(glm::vec3(0.5f, 0.55f, 0.0f));//position of the joint relative to parent
	leftArm->setMeshTranslation(glm::vec3(0.375f, 0.0f, 0.0f));
    leftArm->setMeshScale(glm::vec3(0.75f, 0.4f, 0.4f));

    auto leftLowArm = std::make_shared<Component>(cube, sphere, prog, leftArm);
    leftLowArm->setJointTranslation(glm::vec3(0.75f, 0.0f, 0.0f));
	leftLowArm->setMeshTranslation(glm::vec3(0.375f, 0.0f, 0.0f));
    leftLowArm->setMeshScale(glm::vec3(0.75f, 0.3f, 0.3f));

    auto rightArm = std::make_shared<Component>(cube, sphere, prog, torso);
    rightArm->setJointTranslation(glm::vec3(-0.5f, 0.55f, 0.0f));
	rightArm->setMeshTranslation(glm::vec3(-0.375f, 0.0f, 0.0f));
    rightArm->setMeshScale(glm::vec3(0.75f, 0.4f, 0.4f));

	auto rightLowArm = std::make_shared<Component>(cube, sphere, prog, rightArm, true);
    rightLowArm->setJointTranslation(glm::vec3(-0.75f, 0.0f, 0.0f));
	rightLowArm->setMeshTranslation(glm::vec3(-0.375f, 0.0f, 0.0f));
    rightLowArm->setMeshScale(glm::vec3(0.75f, 0.3f, 0.3f));

    // Create the legs
    auto leftLeg = std::make_shared<Component>(cube, sphere, prog, torso);
    leftLeg->setJointTranslation(glm::vec3(0.25f, -0.75f, 0.0f));
	leftLeg->setMeshTranslation(glm::vec3(0.0f, -0.6f, 0.0f));
    leftLeg->setMeshScale(glm::vec3(0.4f, 1.2f, 0.4f));

	auto leftLowLeg = std::make_shared<Component>(cube, sphere, prog, leftLeg);
    leftLowLeg->setJointTranslation(glm::vec3(0.0f, -1.2f, 0.0f));
	leftLowLeg->setMeshTranslation(glm::vec3(0.0f, -0.5f, 0.0f));
    leftLowLeg->setMeshScale(glm::vec3(0.3f, 1.0f, 0.3f));

    // Create the legs
    auto rightLeg = std::make_shared<Component>(cube, sphere, prog, torso);
    rightLeg->setJointTranslation(glm::vec3(-0.25f, -0.75f, 0.0f));
	rightLeg->setMeshTranslation(glm::vec3(0.0f, -0.6f, 0.0f));
    rightLeg->setMeshScale(glm::vec3(0.4f, 1.2f, 0.4f));

	auto rightLowLeg = std::make_shared<Component>(cube, sphere, prog, rightLeg);
    rightLowLeg->setJointTranslation(glm::vec3(0.0f, -1.2f, 0.0f));
	rightLowLeg->setMeshTranslation(glm::vec3(0.0f, -0.5f, 0.0f));
    rightLowLeg->setMeshScale(glm::vec3(0.3f, 1.0f, 0.3f));

    // You must set the parent of the arms, legs, and head as the torso while also linking them in the constructor
    torso->addChild(head);

    torso->addChild(leftArm);
	leftArm->addChild(leftLowArm);

	torso->addChild(rightArm);
	rightArm->addChild(rightLowArm);

    torso->addChild(leftLeg);
	leftLeg->addChild(leftLowLeg);

	torso->addChild(rightLeg);
	rightLeg->addChild(rightLowLeg);

	currentComponent = torso;
	
	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}

static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width/(float)height;
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create matrix stacks.
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	// Apply projection.
	P->pushMatrix();
	P->multMatrix(glm::perspective((float)(45.0*M_PI/180.0), aspect, 0.01f, 100.0f));
	
	// Apply camera transform.
	MV->pushMatrix();
	MV->translate(glm::vec3(0, 1, -7));
	MV->rotate(M_PI/10, glm::vec3(1, 0, 0));
	
	prog->bind();
		double t = glfwGetTime();
		float a = 0.05;
		float f = 2;
		float s = 1 + (a/2) + (a/2)*sin(2*M_PI*f*t);
		glm::vec3 actualMeshScale = currentComponent->meshScale;
		currentComponent->setMeshScale(s*actualMeshScale);
		torso->draw(MV, P);
		currentComponent->setMeshScale(actualMeshScale);
	prog->unbind();
	
	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

static void char_callback(GLFWwindow *window, unsigned int key){
	switch(key) {
		case '.':
		{
            auto nextChild = currentComponent->getNextChild();
            if (nextChild) {
                currentComponent = nextChild;
            }
			break;
		}
		case ',':
		{
			auto parent = currentComponent->getParent();
            if (parent) {
                currentComponent = parent;
            }
			break;
		}
		case 'x':
		{
			glm::vec3* currentAngles = &(currentComponent->jointAngles);
			currentAngles->x += 0.1;
			break;
		}
		case 'X':
		{
			glm::vec3* currentAngles = &(currentComponent->jointAngles);
			currentAngles->x -= 0.1;
			break;
		}
		case 'y':
		{
			glm::vec3* currentAngles = &(currentComponent->jointAngles);
			currentAngles->y += 0.1;
			break;
		}
		case 'Y':
		{
			glm::vec3* currentAngles = &(currentComponent->jointAngles);
			currentAngles->y -= 0.1;
			break;
		}
		case 'z':
		{
			glm::vec3* currentAngles = &(currentComponent->jointAngles);
			currentAngles->z += 0.1;
			break;
		}
		case 'Z':
		{
			glm::vec3* currentAngles = &(currentComponent->jointAngles);
			currentAngles->z -= 0.1;
			break;
		}
	}
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RES_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// https://en.wikipedia.org/wiki/OpenGL
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "Nitish Malluru", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		if(!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
			// Render scene.
			render();
			// Swap front and back buffers.
			glfwSwapBuffers(window);
		}
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
