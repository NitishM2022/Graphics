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
#include "Texture.h"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;

shared_ptr<Camera> camera;
shared_ptr<Camera> topDownCamera;

vector<Light> lights;
int currentLight = 0;

vector<shared_ptr<Program>> prog;
int currentProg = 1;

shared_ptr<Program> texture;
shared_ptr<Texture> texture0;

vector<Material> materials;

shared_ptr<Shape> bunny;
shared_ptr<Shape> teapot;
shared_ptr<Shape> sphere;
shared_ptr<Shape> plane;
shared_ptr<Shape> frustum;

bool topDown = false;

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

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	static float lastX = static_cast<float>(xmouse);
    static float lastY = static_cast<float>(ymouse);

    float xoffset = static_cast<float>(xmouse) - lastX;
    float yoffset = lastY - static_cast<float>(ymouse); // Reversed since y-coordinates go from bottom to top

    lastX = static_cast<float>(xmouse);
    lastY = static_cast<float>(ymouse);

    camera->mouseMoved(xoffset, yoffset);
}

static void char_callback(GLFWwindow *window, unsigned int key){
	keyToggles[key] = !keyToggles[key];
	switch(key) {
		case 'y':
		{
			if(currentProg == 0){
				currentProg = prog.size() - 1;
			}else{
				currentProg--;
			}
			break;
		}
		case 'w':
		{
            camera->moveForward(0.1f); 
            break;
		}
		case 's':
		{
             camera->moveBackward(0.1f);
            break;
		}
		case 'a':
        {
		    camera->moveLeft(0.1f);
            break;
		}
		case 'd':
        {
		    camera->moveRight(0.1f);
            break;
		}
		case 'z':
        {
		    camera->zoomIn(0.1f); 
            break;
		}
		case 'Z':
		{
            camera->zoomOut(0.1f);
            break;
		}
		case 't':
		{
			topDown = !topDown;
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

static void makeTexture(string vert, string frag, shared_ptr<Program>& progEl)
{
	progEl = make_shared<Program>();
	progEl->setShaderNames(RESOURCE_DIR + vert, RESOURCE_DIR + frag);
	progEl->setVerbose(true);
	progEl->init();
	progEl->addAttribute("aPos");
	progEl->addAttribute("aNor");
	progEl->addAttribute("aTex");

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
	progEl->addUniform("texture0");
	progEl->setVerbose(false);
}

float randFloat() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// Initialize time.
	glfwSetTime(0.0);
	// Set background color.
	glClearColor(0.53f, 0.81f, 0.98f, 1.0f);

	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Create Materials and Lights
	for (int i = 0; i < 100; i++) {
		materials.push_back(
			Material(
				glm::vec3(randFloat(), randFloat(), randFloat()),
				glm::vec3(randFloat(), randFloat(), randFloat()),
				glm::vec3(randFloat(), randFloat(), randFloat()),
				randFloat() * 200.0f
			)
		);
	}
	materials.push_back(Material(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 50.0f)); // Sun
	materials.push_back(Material(glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(1.0f, 1.0f, 1.0f), 20.0f)); // ground

	Light light1 = Light(1, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.8f)); // Sun
	lights.push_back(light1);

	// Initialize the GLSL program. 
	shared_ptr<Program> normal;
	makeNormal("normal_vert.glsl", "normal_frag.glsl", normal);
	prog.push_back(normal);

	shared_ptr<Program> blinn_phong;
	makeBlinnPhong("blinn_phong_vert.glsl", "blinn_phong_frag.glsl", blinn_phong);
	prog.push_back(blinn_phong);

	texture = shared_ptr<Program>();
	makeTexture("texture_vert.glsl", "texture_frag.glsl", texture);
	texture0 = make_shared<Texture>();
	texture0->setFilename(RESOURCE_DIR + "earthKd.jpg");
	texture0->init();
	texture0->setUnit(0);
	texture0->setWrapModes(GL_REPEAT, GL_REPEAT);

	
	camera = make_shared<Camera>();
	camera->setInitDistance(2.0f); // Camera's initial Z translation
	camera->setInitHeight(0.3f);

	topDownCamera = make_shared<Camera>();
	
	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunny->init();

	teapot = make_shared<Shape>();
	teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapot->init();

	sphere = make_shared<Shape>();
	sphere->loadMesh(RESOURCE_DIR + "sphere.obj");
	sphere->init();

	plane = make_shared<Shape>();
	plane->loadMesh(RESOURCE_DIR + "plane.obj");
	plane->init();

	frustum = make_shared<Shape>();
	frustum->loadMesh(RESOURCE_DIR + "frustum.obj");
	frustum->init();

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
	if(keyToggles[(unsigned)'l']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	camera->setAspect((float)width/(float)height);

	
	double t = glfwGetTime();

	float a = (float)width / (float)height;
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	prog[currentProg]->bind();
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();

	// Apply camera transform.
	MV->pushMatrix();
	float groundScale = 10.0f;
	float scale = 0.4f;

	camera->applyViewMatrix(MV);
	glUniformMatrix4fv(prog[currentProg]->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	if(currentProg == 1){
		glUniform3f(prog[currentProg]->getUniform("camPos"), 0.0f, 0.0f, 0.0f); // camPos is always at the center of Camera Space
		{
			MV->pushMatrix();
				MV->translate(lights[0].position);
				MV->scale(scale);
				glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glm::mat4 inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
				glUniformMatrix4fv(prog[currentProg]->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));

				materials[100].setUniforms(prog[currentProg]);
				sphere->draw(prog[currentProg]);
			MV->popMatrix();
		}
		glUniform3fv(prog[currentProg]->getUniform(("lightPos1")), 1, glm::value_ptr(glm::vec3(MV->topMatrix() * glm::vec4(lights[0].position, 1.0f))));
		glUniform3fv(prog[currentProg]->getUniform(("lightCol1")), 1, glm::value_ptr(lights[0].color));
	}else{
		{
			MV->pushMatrix();
				MV->translate(lights[0].position);
				MV->scale(scale);
				glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

				sphere->draw(prog[currentProg]);
			MV->popMatrix();
		}
		{
			MV->pushMatrix();
				MV->scale(groundScale);
				glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

				plane->draw(prog[currentProg]);
			MV->popMatrix();
		}
	}

	for (int i = 0; i < 100; i++) {
		materials[i].setUniforms(prog[currentProg]);
		MV->pushMatrix();
		float x = static_cast<float>(i % 10) - 4.5f;
		float z = static_cast<float>(i / 10) - 4.5f;

		bool shape = false;
		float minY = 0.0f;
		if(i % 2 == 0){
			if((i / 10) % 2 == 1){
				minY = bunny->getMinY();
			}
			else{
				minY = teapot->getMinY();
				shape = true;
			}
		}
		else{
			if((i / 10) % 2 == 0){
				minY = bunny->getMinY();
			}
			else{
				minY = teapot->getMinY();
				shape = true;
			}
		}


		float clampScale = 0.1 * sin(t) + 0.4;
		if(shape){
			clampScale = 0.1 * cos(t) + 0.4;
		}
		
		MV->translate(x, -minY * clampScale , z);
		MV->scale(clampScale);

		glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glm::mat4 inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
		glUniformMatrix4fv(prog[currentProg]->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));

		if(shape){
			teapot->draw(prog[currentProg]);
		}
		else{
			bunny->draw(prog[currentProg]);
		}
		MV->popMatrix();
	}
		
	MV->popMatrix();
	P->popMatrix();

	// HUD
	if(!topDown){
		P->pushMatrix();
		float orthoHUD = 1.5f;
		P->multMatrix(glm::ortho(-orthoHUD * a, orthoHUD * a, -orthoHUD, orthoHUD, 0.1f, 2000.0f));
		glUniformMatrix4fv(prog[currentProg]->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

		float hudPosition = 0.8f;
		float hudScale = 0.5f;
		{
			MV->pushMatrix();
				MV->translate(glm::vec3(-hudPosition * a, hudPosition - bunny->getMinY() * hudScale, -1.0f));
				MV->translate(-0.4f, -0.15f, 0.0f);
				MV->rotate(t, 0.0f, 1.0f, 0.0f);
				MV->scale(hudScale);
				if(currentProg == 1){
					glUniform3f(prog[currentProg]->getUniform("camPos"), 0.0f, 0.0f, 0.0f);
					materials[101].setUniforms(prog[currentProg]);
				}
				glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glm::mat4 inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
				glUniformMatrix4fv(prog[currentProg]->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));

				bunny->draw(prog[currentProg]);
			MV->popMatrix();
		}
		{
			MV->pushMatrix();
				MV->translate(glm::vec3(hudPosition * a, hudPosition - teapot->getMinY() * hudScale, -1.0f));
				MV->translate(0.3f, -0.05f, 0.0f);
				MV->rotate(t, 0.0f, 0.1f, 0.0f);
				MV->scale(hudScale);
				if(currentProg == 1){
					glUniform3f(prog[currentProg]->getUniform("camPos"), 0.0f, 0.0f, 0.0f);
					materials[101].setUniforms(prog[currentProg]);
				}
				glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glm::mat4 inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
				glUniformMatrix4fv(prog[currentProg]->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));
				teapot->draw(prog[currentProg]);
			MV->popMatrix();
		}
		P->popMatrix();
	}	
	prog[currentProg]->unbind();

	// Texture
	texture->bind();
		texture0->bind(texture->getUniform("texture0"));
		P->pushMatrix();
		camera->applyProjectionMatrix(P);
		MV->pushMatrix();
		camera->applyViewMatrix(MV);
		glUniformMatrix4fv(texture->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(texture->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniform3f(texture->getUniform("camPos"), 0.0f, 0.0f, 0.0f);

		MV->pushMatrix();
			MV->rotate(-M_PI, 1.0f, 0.0f, 0.0f);
			MV->rotate(-M_PI, 0.0f, 0.0f, 1.0f);
			MV->scale(groundScale);
			glUniformMatrix4fv(texture->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glm::mat4 inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
			glUniformMatrix4fv(texture->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));
			glUniform3fv(texture->getUniform(("lightPos1")), 1, glm::value_ptr(glm::vec3(MV->topMatrix() * glm::vec4(lights[0].position, 1.0f))));
			glUniform3fv(texture->getUniform(("lightCol1")), 1, glm::value_ptr(lights[0].color));
			materials[101].setUniforms(texture);
			plane->draw(texture);
		MV->popMatrix();
		MV->popMatrix();
		P->popMatrix();
		texture0->unbind();
	texture->unbind();

	// MiniMap
	if(topDown){
		double s = 0.5;
		glViewport(0, 0, s*width, s*height);
		topDownCamera->setAspect((float)s*width/(float)s*height);
		glEnable(GL_SCISSOR_TEST);
		glScissor(0, 0, s*width, s*height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		P->pushMatrix();
		MV->pushMatrix();
			P->multMatrix(glm::ortho(-6.0f * a, 6.0f * a, 6.0f, -6.0f, 0.1f, 1000.0f));
			MV->translate(0.0f, -2.0f, 0.0f);
			MV->rotate(-M_PI/2.0f, 1.0f, 0.0f, 0.0f);
			MV->translate(0.0f, 0.0f, 2.0f);// to offset orginal camera position
			topDownCamera->applyViewMatrix(MV);
			texture->bind();
			texture0->bind(texture->getUniform("texture0"));
				glUniformMatrix4fv(texture->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
				glUniformMatrix4fv(texture->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniform3f(texture->getUniform("camPos"), 0.0f, 0.0f, 0.0f);

				MV->pushMatrix();
					MV->translate(0.0f, 1.0f, 0.0f);
					MV->rotate(-M_PI, 1.0f, 0.0f, 0.0f);
					MV->rotate(-M_PI, 0.0f, 0.0f, 1.0f);
					MV->scale(groundScale);
					glUniformMatrix4fv(texture->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
					glm::mat4 inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
					glUniformMatrix4fv(texture->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));
					glUniform3fv(texture->getUniform(("lightPos1")), 1, glm::value_ptr(glm::vec3(MV->topMatrix() * glm::vec4(lights[0].position, 1.0f))));
					glUniform3fv(texture->getUniform(("lightCol1")), 1, glm::value_ptr(lights[0].color));
					materials[101].setUniforms(texture);
					plane->draw(texture);
				MV->popMatrix();
			texture0->unbind();
			texture->unbind();
			
			prog[currentProg]->bind();
			glUniformMatrix4fv(prog[currentProg]->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			if(currentProg == 1){
				glUniform3fv(prog[currentProg]->getUniform("camPos"), 1, glm::value_ptr(camera->getCameraPosition()));
				{
					MV->pushMatrix();
						MV->translate(lights[0].position);
						MV->scale(scale);
						glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
						materials[100].setUniforms(prog[currentProg]);
						sphere->draw(prog[currentProg]);
					MV->popMatrix();
				}
				
				glUniform3fv(prog[currentProg]->getUniform(("lightPos1")), 1, glm::value_ptr(glm::vec3(MV->topMatrix() * glm::vec4(lights[0].position, 1.0f))));
				glUniform3fv(prog[currentProg]->getUniform(("lightCol1")), 1, glm::value_ptr(lights[0].color));
			}else{
				MV->pushMatrix();
					MV->translate(lights[0].position);
					MV->scale(scale);
					glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
					sphere->draw(prog[currentProg]);
				MV->popMatrix();
			}
			for (int i = 0; i < 100; i++) {
				materials[i].setUniforms(prog[currentProg]);
				MV->pushMatrix();
				float x = static_cast<float>(i % 10) - 4.5f;
				float z = static_cast<float>(i / 10) - 4.5f;

				bool shape = false;
				float minY = 0.0f;
				if(i % 2 == 0){
					if((i / 10) % 2 == 1){
						minY = bunny->getMinY();
					}
					else{
						minY = teapot->getMinY();
						shape = true;
					}
				}
				else{
					if((i / 10) % 2 == 0){
						minY = bunny->getMinY();
					}
					else{
						minY = teapot->getMinY();
						shape = true;
					}
				}

				float clampScale = 0.1 * sin(t) + 0.4;
				if(shape){
					clampScale = 0.1 * cos(t) + 0.6;
				}

				MV->translate(x, -minY * clampScale , z);
				MV->scale(clampScale);

				glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glm::mat4 inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
				glUniformMatrix4fv(prog[currentProg]->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));

				if(shape){
					teapot->draw(prog[currentProg]);
				}
				else{
					bunny->draw(prog[currentProg]);
				}
				MV->popMatrix();
			}

			//draw fructum
			glDisable(GL_DEPTH_TEST);
			MV->pushMatrix();
				float sx = a * tan(camera->getFovy() / 2.0f);
				float sy = tan(camera->getFovy() / 2.0f);
				glm::mat4 cameraMatrix = camera->getCameraMatrix();
				MV->multMatrix(cameraMatrix);
				MV->scale(sx, sy, 1.0f);
				glUniformMatrix4fv(prog[currentProg]->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				if(currentProg == 1){
					materials[100].setUniforms(prog[currentProg]);
				}
				frustum->draw(prog[currentProg]);
			MV->popMatrix();
			glEnable(GL_DEPTH_TEST);

			prog[currentProg]->unbind();
		MV->popMatrix();
    	P->popMatrix();
	}


	
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
