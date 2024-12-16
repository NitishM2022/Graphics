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

// textures
GLuint framebufferID;
GLuint posTexture;
GLuint norTexture;
GLuint keTexture;
GLuint kdTexture;
bool blur = false;

shared_ptr<Camera> camera;
shared_ptr<Camera> cameraPass2;

map<string,GLuint> bufID1;
int indCount1;

map<string,GLuint> bufID2;
int indCount2;

int lightCount = 20;
vector<glm::vec3> lightPos;
vector<glm::vec3> lightCol;

int currentProg = 0;
vector<shared_ptr<Program>> prog;
shared_ptr<Program> pass1;
shared_ptr<Program> pass2;

vector<Material> materials;
vector<Material> lightMaterials;

shared_ptr<Shape> bunny;
shared_ptr<Shape> teapot;

float lightScale = 0.02f;
shared_ptr<Shape> sphere;

float groundScale = 6.0f;
shared_ptr<Shape> plane;

vector<float> scales;

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
			if(currentProg == prog.size() - 1){
				currentProg = 0;
			}else{
				currentProg++;
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
		case 'b':
		{
			blur = !blur;
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

static void makePass1(string vert, string frag, shared_ptr<Program>& progEl)
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

	progEl->addUniform("SOR");
	progEl->addUniform("t");

	progEl->addUniform("amb");
	progEl->addUniform("diff");
	
	progEl->setVerbose(false);
}


static void makePass2(string vert, string frag, shared_ptr<Program>& progEl)
{
	progEl = make_shared<Program>();
	progEl->setShaderNames(RESOURCE_DIR + vert, RESOURCE_DIR + frag);
	progEl->setVerbose(true);
	progEl->init();

	progEl->addAttribute("aPos");
	progEl->addUniform("MV");
	progEl->addUniform("P");

	progEl->addUniform("posTexture");
	progEl->addUniform("norTexture");
	progEl->addUniform("keTexture");
	progEl->addUniform("kdTexture");
	progEl->addUniform("windowSize");
	progEl->addUniform("blur");
	
	progEl->addUniform("lightPos");
	progEl->addUniform("lightCol");
	progEl->addUniform("camPos");

	progEl->setVerbose(false);
}

float randFloat() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float scaleRandFloat() {
    float min = 0.3f;
    float max = 0.6f;
    return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX/(max - min));
}

void initSquashSphere() {
	vector<float> posBuf;
	vector<float> norBuf;
	vector<unsigned int> indBuf;

	int thetaSteps = 50;
	int phiSteps = 50;
	for (int i = 0; i <= thetaSteps; i++) {
		for (int j = 0; j <= phiSteps; j++) {
			float theta = static_cast<float>(i) / static_cast<float>(thetaSteps) * M_PI;
			float phi = static_cast<float>(j) / static_cast<float>(phiSteps) * 2.0f * M_PI;
			float x = sin(theta) * sin(phi);
			float y = cos(theta);
			float z = sin(theta) * cos(phi);
			posBuf.push_back(x);
			posBuf.push_back(y);
			posBuf.push_back(z);
			norBuf.push_back(x);
			norBuf.push_back(y);
			norBuf.push_back(z);
		}
	}

	for (int i = 0; i < thetaSteps; i++) {
		for (int j = 0; j < phiSteps; j++) {
			indBuf.push_back(i * (phiSteps + 1) + j);
			indBuf.push_back((i + 1) * (phiSteps + 1) + j);
			indBuf.push_back((i + 1) * (phiSteps + 1) + j + 1);
			indBuf.push_back(i * (phiSteps + 1) + j);
			indBuf.push_back((i + 1) * (phiSteps + 1) + j + 1);
			indBuf.push_back(i * (phiSteps + 1) + j + 1);
		}
	}
	
	indCount1 = (int)indBuf.size();
		
	GLuint tmp[3];
	glGenBuffers(3, tmp);
	bufID1["bPos"] = tmp[0];
	bufID1["bNor"] = tmp[1];
	bufID1["bInd"] = tmp[2];
	glBindBuffer(GL_ARRAY_BUFFER, bufID1["bPos"]);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, bufID1["bNor"]);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufID1["bInd"]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(norBuf.size() == posBuf.size());
}

void drawSquashSphere(const shared_ptr<Program> prog, const shared_ptr<MatrixStack> P, const shared_ptr<MatrixStack> MV){
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
	glEnableVertexAttribArray(prog->getAttribute("aPos"));
	GLSL::checkError(GET_FILE_LINE);
	glEnableVertexAttribArray(prog->getAttribute("aNor"));
	GLSL::checkError(GET_FILE_LINE);
	glBindBuffer(GL_ARRAY_BUFFER, bufID1["bPos"]);
	glVertexAttribPointer(prog->getAttribute("aPos"), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, bufID1["bNor"]);
	glVertexAttribPointer(prog->getAttribute("aNor"), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufID1["bInd"]);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
	glDrawElements(GL_TRIANGLES, indCount1, GL_UNSIGNED_INT, (void *)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(prog->getAttribute("aNor"));
	glDisableVertexAttribArray(prog->getAttribute("aPos"));
}


static void initSOR()
{	
	vector<float> posBuf;
	vector<float> norBuf;
	vector<unsigned int> indBuf;

	int steps = 50;
	int domain = 10;
	for (int i = 0; i < steps; i++) {
		for (int j = 0; j < steps; j++) {
			float x = domain * (float)i / (steps - 1); 
			float theta = 2 * M_PI * (float)j / (steps - 1);

			posBuf.push_back(x);
			posBuf.push_back(theta);
			posBuf.push_back(0.0f);
		
			norBuf.push_back(0.0f);
			norBuf.push_back(0.0f);
			norBuf.push_back(0.0f);
		}
	}

	for (int i = 0; i < steps - 1; i++) {
		for (int j = 0; j < steps - 1; j++) {
			unsigned int bottomLeft = i * steps + j;
			unsigned int bottomRight = bottomLeft + 1;
			unsigned int topLeft = bottomLeft + steps;
			unsigned int topRight = topLeft + 1;

			// First triangle (bottomLeft, topLeft, topRight)
			indBuf.push_back(bottomLeft);
			indBuf.push_back(topLeft);
			indBuf.push_back(topRight);

			// Second triangle (bottomLeft, topRight, bottomRight)
			indBuf.push_back(bottomLeft);
			indBuf.push_back(topRight);
			indBuf.push_back(bottomRight);
		}
	}
	
	indCount2 = (int)indBuf.size();
		
	GLuint tmp[3];
	glGenBuffers(3, tmp);
	bufID2["bPos"] = tmp[0];
	bufID2["bNor"] = tmp[1];
	bufID2["bInd"] = tmp[2];
	glBindBuffer(GL_ARRAY_BUFFER, bufID2["bPos"]);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, bufID2["bNor"]);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufID2["bInd"]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void drawSOR(const shared_ptr<Program> prog, const shared_ptr<MatrixStack> P, const shared_ptr<MatrixStack> MV){
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
	glEnableVertexAttribArray(prog->getAttribute("aPos"));
	GLSL::checkError(GET_FILE_LINE);
	glEnableVertexAttribArray(prog->getAttribute("aNor"));
	GLSL::checkError(GET_FILE_LINE);
	glBindBuffer(GL_ARRAY_BUFFER, bufID2["bPos"]);
	glVertexAttribPointer(prog->getAttribute("aPos"), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, bufID2["bNor"]);
	glVertexAttribPointer(prog->getAttribute("aNor"), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufID2["bInd"]);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
	glDrawElements(GL_TRIANGLES, indCount2, GL_UNSIGNED_INT, (void *)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(prog->getAttribute("aNor"));
	glDisableVertexAttribArray(prog->getAttribute("aPos"));
}

static void initTexture(int width, int height){
	glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

	glGenTextures(1, &posTexture);
	glBindTexture(GL_TEXTURE_2D, posTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTexture, 0);

	glGenTextures(1, &norTexture);
	glBindTexture(GL_TEXTURE_2D, norTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, norTexture, 0);

	glGenTextures(1, &keTexture);
	glBindTexture(GL_TEXTURE_2D, keTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, keTexture, 0);

	glGenTextures(1, &kdTexture);
	glBindTexture(GL_TEXTURE_2D, kdTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, kdTexture, 0);

	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	GLenum DrawBuffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, DrawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "Framebuffer not complete!" << endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// Initialize time.
	glfwSetTime(0.0);
	// Set background color.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Create Materials and Lights
	for (int i = 0; i < 100; i++) {
		materials.push_back(
			Material(
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(randFloat(), randFloat(), randFloat()),
				glm::vec3(1.0f, 1.0f, 1.0f),
				10.0f
			)
		);
	}
	materials.push_back(Material(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), 20.0f)); // ground

	float angleStep = 2.0f * M_PI / lightCount;
	for(int i = 0; i < lightCount; i++){
		float angle = i * angleStep;
		glm::vec3 pos(groundScale / 2 * cos(angle), 0.35f, groundScale / 2 * sin(angle));
		lightPos.push_back(pos);
		lightCol.push_back(glm::vec3(randFloat(), randFloat(), randFloat()));
		lightMaterials.push_back(Material(lightCol[i], glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f)); 
	}

	// Initialize the GLSL program. 
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	initTexture(width, height);

	makePass1("pass1_vert.glsl", "pass1_frag.glsl", pass1);
	makePass2("pass2_vert.glsl", "pass2_frag.glsl", pass2);
	pass2->bind();
		glUniform1i(pass2->getUniform("posTexture"), 0);
		glUniform1i(pass2->getUniform("norTexture"), 1);
		glUniform1i(pass2->getUniform("keTexture"), 2);
		glUniform1i(pass2->getUniform("kdTexture"), 3);
	pass2->unbind();

	
	camera = make_shared<Camera>();
	camera->setInitDistance(2.0f); // Camera's initial Z translation
	camera->setInitHeight(0.5f);

	cameraPass2 = make_shared<Camera>();
	cameraPass2->setInitDistance(0.2f); 
	
	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunny->init();

	teapot = make_shared<Shape>();
	teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapot->init();

	sphere = make_shared<Shape>();
	sphere->loadMesh(RESOURCE_DIR + "sphere2.obj");
	sphere->init();

	plane = make_shared<Shape>();
	plane->loadMesh(RESOURCE_DIR + "plane.obj");
	plane->init();

	initSquashSphere();
	initSOR();

	for (int i = 0; i < 100; i++) {
		scales.push_back(scaleRandFloat());
	}

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
	vector<glm::vec3> transformedLightPos;
	glfwGetFramebufferSize(window, &width, &height);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	glViewport(0, 0, width, height);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera->setAspect((float)width/(float)height);
	
	double t = glfwGetTime();

	float a = (float)width / (float)height;
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	pass1->bind();
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);

	// Apply camera transform.
	MV->pushMatrix();

	camera->applyViewMatrix(MV);
	glUniformMatrix4fv(pass1->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(pass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));	

	glUniform3f(pass1->getUniform("camPos"), 0.0f, 0.0f, 0.0f); // camPos is always at the center of Camera Space
	// draw lights
	for(int i = 0; i < lightCount; i++){
		MV->pushMatrix();
				MV->translate(lightPos[i].x, lightPos[i].y, lightPos[i].z);
				MV->scale(lightScale);
				glUniformMatrix4fv(pass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glm::mat4 inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
				glUniformMatrix4fv(pass1->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));

				lightMaterials[i].setUniforms(pass1);
				sphere->draw(pass1);
		MV->popMatrix();
		transformedLightPos.push_back(glm::vec3(MV->topMatrix() * glm::vec4(lightPos[i], 1.0f)));
	}	
		// draw ground
		{
			MV->pushMatrix();
				MV->scale(groundScale);
				glUniformMatrix4fv(pass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				
				materials[100].setUniforms(pass1);
				plane->draw(pass1);
			MV->popMatrix();
		}


	// draw objects
	for (int i = 0; i < 100; i++) {
		materials[i].setUniforms(pass1);
		

		MV->pushMatrix();

		float x = static_cast<float>(i % 10) - 4.5f;
		float z = static_cast<float>(i / 10) - 4.5f;

		float objectScale = scales[i];
		std::shared_ptr<Shape> objectToDraw;

		switch (i % 4) {
			case 0:
			{
				objectToDraw = bunny;
				objectScale *= 0.85f;
				MV->translate(x, -bunny->getMinY() * objectScale, z);
				MV->rotate(i % 8 == 0 ? t : -t, glm::vec3(0.0f, 1.0f, 0.0f));
				MV->scale(objectScale);
				break;
			}
			case 1:
			{
				objectToDraw = teapot;
				MV->translate(x, -teapot->getMinY() * objectScale, z);
				glm::mat4 shearMatrix = glm::mat4(1.0f); 
				shearMatrix[1][0] = 0.3f * sin(t * 1.5);
				MV->multMatrix(shearMatrix);
				MV->scale(objectScale);
				break;
			}
			case 2:
			{
				double Ay = 0.5;
				double As = 0.5;
				double p = 1.7;
				double t0 = 0.9;
				double y = Ay * (0.5 * sin(2 * M_PI / p * (t + t0)) + 0.5);
			    double s = -As * (0.5 * cos(4 * M_PI / p * (t + t0)) + 0.5) + 1;

				objectScale *= 0.6f;
				MV->translate(x, y + -sphere->getMinY() * objectScale, z);
				MV->scale(objectScale * s, objectScale * 1.0f, objectScale * s);
				break;
			}
			case 3:
			{
				glUniform1f(pass1->getUniform("t"), static_cast<float>(t));
				objectScale *= 0.6f;
				MV->translate(x, 0.0f, z);
				MV->rotate(M_PI/2, glm::vec3(0.0f, 0.0f, 1.0f));
				MV->scale(scales[i] * 0.2);
				break;
			}
		}

		glUniformMatrix4fv(pass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glm::mat4 inverseTransposeMV = glm::transpose(glm::inverse(MV->topMatrix()));
		glUniformMatrix4fv(pass1->getUniform("inverseTransposeMV"), 1, GL_FALSE, glm::value_ptr(inverseTransposeMV));

		if (i % 4 == 2) {
			drawSquashSphere(pass1, P, MV);
		} else if(i % 4 == 3){
			glUniform1i(pass1->getUniform("SOR"), true);
			drawSOR(pass1, P, MV);
			glUniform1i(pass1->getUniform("SOR"), false);
		} else if (objectToDraw != nullptr) {
			objectToDraw->draw(pass1);
		}

		MV->popMatrix();
	}
			
	MV->popMatrix();
	P->popMatrix();
	pass1->unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    pass2->bind();
    P->pushMatrix();
    cameraPass2->applyProjectionMatrix(P);
    MV->pushMatrix();
    cameraPass2->applyViewMatrix(MV);

    // Bind the G-buffer textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, posTexture);
    glUniform1i(pass2->getUniform("posTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, norTexture);
    glUniform1i(pass2->getUniform("norTexture"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, keTexture);
    glUniform1i(pass2->getUniform("keTexture"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, kdTexture);
    glUniform1i(pass2->getUniform("kdTexture"), 3);

	glUniform3f(pass2->getUniform("camPos"), 0.0f, 0.0f, 0.0f); // camPos is always at the center of Camera Space

	glUniform3fv(pass2->getUniform("lightPos"), lightCount, glm::value_ptr(transformedLightPos[0]));
	glUniform3fv(pass2->getUniform("lightCol"), lightCount, glm::value_ptr(lightCol[0]));

	// Render a view-aligned quad
	MV->rotate(M_PI/2, glm::vec3(1.0f, 0.0f, 0.0f)); 
    MV->scale(a, 1.0f, a); // Scale the quad to cover the screen

	glUniformMatrix4fv(pass2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
    glUniformMatrix4fv(pass2->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    glUniform2f(pass2->getUniform("windowSize"), static_cast<float>(width), static_cast<float>(height));
	glUniform1i(pass2->getUniform("blur"), blur);

	plane->draw(pass2);

    MV->popMatrix();
    P->popMatrix();
   	pass2->unbind();
	transformedLightPos.clear();
	
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
