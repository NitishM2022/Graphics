#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Material.h"
#include "Light.h"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;

shared_ptr<Camera> camera;

vector<Light> lights;
int currentLight = 0;

vector<shared_ptr<Program>> prog;
int currentProg = 0;

vector<Material> materials;
int currentMaterial = 0;

shared_ptr<Shape> bunny;
shared_ptr<Shape> teapot;

bool keyToggles[256] = {false}; // only for English keyboards!

// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

// This function is called when the mouse is clicked
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = (mods & GLFW_MOD_SHIFT) != 0;
		bool ctrl  = (mods & GLFW_MOD_CONTROL) != 0;
		bool alt   = (mods & GLFW_MOD_ALT) != 0;
		camera->mouseClicked((float)xmouse, (float)ymouse, shift, ctrl, alt);
	}
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		camera->mouseMoved((float)xmouse, (float)ymouse);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key){
	keyToggles[key] = !keyToggles[key];
	switch(key) {
		case 's':
		{
			if(currentProg == 0){
				currentProg = prog.size() - 1;
			}else{
				currentProg--;
			}
			break;
		}
		case 'S':
		{
			if(currentProg == prog.size() - 1){
				currentProg = 0;
			}else{
				currentProg++;
			}
			break;
		}
		case 'm':
		{	
			if(currentMaterial == 0){
				currentMaterial = materials.size() - 1;
			}else{
				currentMaterial--;
			}
			break;
		}
		case 'M':
		{
			if(currentMaterial == materials.size() - 1){
				currentMaterial = 0;
			}else{
				currentMaterial++;
			}
			break;
		}
		case 'l':
		{
			if (currentLight == 0) {
				currentLight = lights.size() - 1;
			} else {
				currentLight--;
			}
			break;
		}
		case 'L':
		{
			if (currentLight == lights.size() - 1) {
				currentLight = 0;
			} else {
				currentLight++;
			}
			break;
		}
		case 'x':
		{
			lights[currentLight].position.x -= 0.1f;
			break;
		}
		case 'X':
		{
			lights[currentLight].position.x += 0.1f;
			break;
		}
		case 'y':
		{
			lights[currentLight].position.y -= 0.1f;
			break;
		}
		case 'Y':
		{
			lights[currentLight].position.y += 0.1f;
			break;
		}
	}
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
static void saveImage(const char *filepath, GLFWwindow *w)
{
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	int rc = stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
	if(rc) {
		cout << "Wrote to " << filepath << endl;
	} else {
		cout << "Couldn't write to " << filepath << endl;
	}
}

static void makeNormal(string vert, string frag, shared_ptr<Program>& progEl)
{
	progEl = make_shared<Program>();
	progEl->setShaderNames(RESOURCE_DIR + vert, RESOURCE_DIR + frag);
	progEl->setVerbose(true);
	progEl->init();
	progEl->addAttribute("aPos");
	progEl->addAttribute("aNor");
	progEl->addUniform("MV");
	progEl->addUniform("P");
	progEl->setVerbose(false);
}

static void makeBlinnPhong(string vert, string frag, shared_ptr<Program>& progEl)
{
	progEl = make_shared<Program>();
	progEl->setShaderNames(RESOURCE_DIR + vert, RESOURCE_DIR + frag);
	progEl->setVerbose(true);
	progEl->init();
	progEl->addAttribute("aPos");
	progEl->addAttribute("aNor");

	progEl->addUniform("MV");
	progEl->addUniform("P");
	progEl->addUniform("inverseTransposeMV");

	progEl->addUniform("amb");
	progEl->addUniform("diff");
	progEl->addUniform("spec");
	progEl->addUniform("shiny");

	for(int i = 1; i <= lights.size(); i++){
		progEl->addUniform("lightPos" + to_string(i));
		progEl->addUniform("lightCol" + to_string(i));
	}
	progEl->addUniform("camPos");
	progEl->setVerbose(false);
}

static void makeSil(string vert, string frag, shared_ptr<Program>& progEl)
{
	progEl = make_shared<Program>();
	progEl->setShaderNames(RESOURCE_DIR + vert, RESOURCE_DIR + frag);
	progEl->setVerbose(true);
	progEl->init();
	progEl->addAttribute("aPos");
	progEl->addAttribute("aNor");

	progEl->addUniform("MV");
	progEl->addUniform("P");
	progEl->addUniform("inverseTransposeMV");

	progEl->addUniform("camPos");
	progEl->setVerbose(false);
}

static void makeCel(string vert, string frag, shared_ptr<Program>& progEl)
{
	progEl = make_shared<Program>();
	progEl->setShaderNames(RESOURCE_DIR + vert, RESOURCE_DIR + frag);
	progEl->setVerbose(true);
	progEl->init();
	progEl->addAttribute("aPos");
	progEl->addAttribute("aNor");

	progEl->addUniform("MV");
	progEl->addUniform("P");
	progEl->addUniform("inverseTransposeMV");

	progEl->addUniform("amb");
	progEl->addUniform("diff");
	progEl->addUniform("spec");
	progEl->addUniform("shiny");

	for(int i = 1; i <= lights.size(); i++){
		progEl->addUniform("lightPos" + to_string(i));
		progEl->addUniform("lightCol" + to_string(i));
	}
	progEl->addUniform("camPos");
	progEl->setVerbose(false);
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// Initialize time.
	glfwSetTime(0.0);
	
	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Create Materials and Lights
	Material first =  Material(glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.8f, 0.7f, 0.7f), glm::vec3(1.0f, 0.9f, 0.8f), 200.0f);
	Material second =  Material(glm::vec3(0.0f, 0.0f, 0.4f), glm::vec3(0.0f, 0.0f, 0.7f), glm::vec3(0.0f, 1.0f, 0.0f), 100.0f);
	Material third =  Material(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.55f, 0.55f, 0.65f), glm::vec3(0.2f, 0.2f, 0.27f), 5.0f);
	materials.push_back(first);
	materials.push_back(second);
	materials.push_back(third);

	Light light1 = Light(1, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.8f, 0.8f, 0.8f));
	Light light2 = Light(2, glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.2f, 0.2f, 0.0f));
	lights.push_back(light1);
	lights.push_back(light2);

	// Initialize the GLSL program. 
	shared_ptr<Program> normal;
	makeNormal("normal_vert.glsl", "normal_frag.glsl", normal);
	prog.push_back(normal);

	shared_ptr<Program> blinn_phong;
	makeBlinnPhong("blinn_phong_vert.glsl", "blinn_phong_frag.glsl", blinn_phong);
	prog.push_back(blinn_phong);

	shared_ptr<Program> sil;
	makeSil("sil_vert.glsl", "sil_frag.glsl", sil);
	prog.push_back(sil);

	shared_ptr<Program> cel;
	makeCel("cel_vert.glsl", "cel_frag.glsl", cel);
	prog.push_back(cel);
	
	camera = make_shared<Camera>();
	camera->setInitDistance(2.0f); // Camera's initial Z translation
	
	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunny->init();

	teapot = make_shared<Shape>();
	teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapot->init();
	
	GLSL::checkError(GET_FILE_LINE);
}

// This function is called every frame to draw the scene.
static void render()
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'z']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	
	double t = glfwGetTime();

	if(!keyToggles[(unsigned)' ']) {
		// Spacebar turns animation on/off
		t = 0.0f;
	}
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);

	// Apply camera transform.
	MV->pushMatrix();


	prog[currentProg]->bind();
	glUniformMatrix4fv(prog[currentProg]->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	if(currentProg == 1 || currentProg == 3){
		glUniform3f(prog[currentProg]->getUniform("camPos"), 0.0f, 0.0f, 0.0f); // camPos is always at the center of Camera Space
		materials[currentMaterial].setUniforms(prog[currentProg]);
		for(int i = 0; i < lights.size(); i++){
			lights[i].setUniforms(prog[currentProg]);
		}
	}else if(currentProg == 2){
		glUniform3f(prog[currentProg]->getUniform("camPos"), 0.0f, 0.0f, 0.0f); 
	}

	MV->pushMatrix();
		MV->translate(glm::vec3(-0.5f, -0.5f, 0.0f));
		MV->rotate(t, 0.0f, 1.0f, 0.0f);
		MV->scale(0.5f);

		glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glm::mat4 inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
		glUniformMatrix4fv(prog[currentProg]->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));
		
		bunny->draw(prog[currentProg]);
	MV->popMatrix();

	MV->pushMatrix();
		MV->translate(glm::vec3(0.5f, 0.0f, 0.0f));

		glm::mat4 S(1.0f);
		S[0][1] = 0.5f*cos(t);
		MV->multMatrix(S);

		MV->rotate(3.14f, glm::vec3(0.0f, 1.0f, 0.0f));
		MV->scale(0.5f);

		glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
		glUniformMatrix4fv(prog[currentProg]->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));

		teapot->draw(prog[currentProg]);
	MV->popMatrix();
	
	prog[currentProg]->unbind();
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
	
	if(OFFLINE) {
		saveImage("output.png", window);
		GLSL::checkError(GET_FILE_LINE);
		glfwSetWindowShouldClose(window, true);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Usage: A3 RESOURCE_DIR" << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Optional argument
	if(argc >= 3) {
		OFFLINE = atoi(argv[2]) != 0;
	}

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
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
	GLSL::checkVersion();
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
