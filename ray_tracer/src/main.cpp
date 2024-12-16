#include <iostream>
#include <string>
#include <cmath>

#include <glm/glm.hpp>

#include "Image.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "common.h"
#include "Sphere.h"
#include "Ellipsoid.h"
#include "Plane.h"
#include "Mesh.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

int width;
int height;

Image* output;
vector<vector<float>> depthBuffer;

glm::vec3 blinnPhongShading(Material mat, vector<Light>& lights, Scene& scene, glm::vec3& origin, glm::vec3& ray, Hit& hit, bool shadow=false, int recursionDepth=4) {
    if (mat.isReflective) {
        if(recursionDepth == 0){
            return glm::vec3(0.0f);
        }else{
            glm::vec3 color = glm::vec3(0.0f);
            glm::vec3 reflectDir = glm::reflect(ray, hit.n);

            Hit reflectHit;
            Material reflectMat;
            bool reflectRayHit = scene.hit(hit.x + 0.001f * reflectDir, reflectDir, reflectHit, reflectMat);

            if (reflectRayHit) {
                return blinnPhongShading(reflectMat, lights, scene, hit.x, reflectDir, reflectHit, shadow, recursionDepth - 1);
            }else{
                return color;
            }
        }
    }
    
    glm::vec3 color = mat.amb;

    for (Light light : lights) {
        Hit shadowHit;
        Material tempMaterial;

        if(shadow){
            bool shadowRayHit = scene.hit(hit.x + 0.001f * hit.n, glm::normalize(light.position - hit.x), shadowHit, tempMaterial);
            if(shadowRayHit && shadowHit.t < glm::length(light.position - hit.x)){
                continue;
            }
        }

        // diffuse
        glm::vec3 lightDir = glm::normalize(light.position - hit.x);
        glm::vec3 diffuse = mat.diff * max(0.0f, glm::dot(hit.n, lightDir));
        // specular
        glm::vec3 viewDir = glm::normalize(origin - hit.x);
        glm::vec3 halfDir = glm::normalize(viewDir + lightDir);
        glm::vec3 specular = mat.spec * pow(max(0.0f, glm::dot(hit.n, halfDir)), mat.exp);

        color += (diffuse + specular) * light.intensity;
    }

    return color;
}

glm::vec3 normalShader(Hit hit) {
    // Convert the normal from [-1, 1] to [0, 1]
    glm::vec3 color = hit.n * 0.5f + 0.5f;
    return color;
}

void scene1(Camera& c, glm::vec3 camPos, glm::mat4& V){
    vector<glm::vec3> rays;
    c.genRays(rays);

    vector<Light> lights;
    Light light1;
    light1.position = glm::vec3(-2.0f, 1.0f, 1.0f);
    light1.intensity = 1.0f;
    lights.push_back(light1);

    // Red Sphere
    Material red;
    red.diff = glm::vec3(1.0f, 0.0f, 0.0f);
    red.spec = glm::vec3(1.0f, 1.0f, 0.5f);
    red.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    red.exp = 300.0f;

    Sphere sphereR(glm::vec3(-0.5, -1.0, 1.0), 1.0f, red);

    // Green Sphere
    Material green;
    green.diff = glm::vec3(0.0f, 1.0f, 0.0f);
    green.spec = glm::vec3(1.0f, 1.0f, 0.5f);
    green.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    green.exp = 300.0f;

    Sphere sphereG(glm::vec3(0.5, -1.0, -1.0), 1.0f, green);

    // Blue Sphere
    Material blue;
    blue.diff = glm::vec3(0.0f, 0.0f, 1.0f); 
    blue.spec = glm::vec3(1.0f, 1.0f, 0.5f); 
    blue.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    blue.exp = 100.0f;

    Sphere sphereB(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, blue); 

    Scene scene;
    scene.addShape(&sphereR);
    scene.addShape(&sphereG);
    scene.addShape(&sphereB);

    // Draw each pixel
    for (int i = 0; i < rays.size(); i++) {
        Hit hit;
        Material hitMaterial;
        bool rayHit = scene.hit(camPos, rays[i], hit, hitMaterial);
        if(rayHit)
        {
            int x = i % width;
            int y = i / width;

            if(hit.t < depthBuffer[x][y]){
                depthBuffer[x][y] = hit.t;
            }else{
                continue;
            }

            glm::vec3 color = blinnPhongShading(hitMaterial, lights, scene, camPos, rays[i], hit);
            output->setPixel(x, y, static_cast<int>(std::min(color.r * 255.0f, 255.0f)), static_cast<int>(std::min(color.g * 255.0f, 255.0f)), static_cast<int>(std::min(color.b * 255.0f, 255.0f)));
        }
    }
}

void scene2(Camera& c, glm::vec3 camPos, glm::mat4& V){
    vector<glm::vec3> rays;
    c.genRays(rays);

    vector<Light> lights;
    Light light1;
    light1.position = glm::vec3(-2.0f, 1.0f, 1.0f);
    light1.intensity = 1.0f;
    lights.push_back(light1);

    // Red Sphere
    Material red;
    red.diff = glm::vec3(1.0f, 0.0f, 0.0f);
    red.spec = glm::vec3(1.0f, 1.0f, 0.5f);
    red.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    red.exp = 100.0f;

    Sphere sphereR(glm::vec3(-0.5, -1.0, 1.0), 1.0f, red);

    // Green Sphere
    Material green;
    green.diff = glm::vec3(0.0f, 1.0f, 0.0f);
    green.spec = glm::vec3(1.0f, 1.0f, 0.5f);
    green.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    green.exp = 100.0f;

    Sphere sphereG(glm::vec3(0.5, -1.0, -1.0), 1.0f, green);

    // Blue Sphere
    Material blue;
    blue.diff = glm::vec3(0.0f, 0.0f, 1.0f); 
    blue.spec = glm::vec3(1.0f, 1.0f, 0.5f); 
    blue.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    blue.exp = 100.0f;

    Sphere sphereB(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, blue); 

    Scene scene;
    scene.addShape(&sphereR);
    scene.addShape(&sphereG);
    scene.addShape(&sphereB);

    // Draw each pixel
    for (int i = 0; i < rays.size(); i++) {
        Hit hit;
        Material hitMaterial;
        bool rayHit = scene.hit(camPos, rays[i], hit, hitMaterial);
        if(rayHit)
        {
            int x = i % width;
            int y = i / width;

            if(hit.t < depthBuffer[x][y]){
                depthBuffer[x][y] = hit.t;
            }else{
                continue;
            }
            glm::vec3 color = blinnPhongShading(hitMaterial, lights, scene, camPos, rays[i], hit, true);
            output->setPixel(x, y, static_cast<int>(std::min(color.r * 255.0f, 255.0f)), static_cast<int>(std::min(color.g * 255.0f, 255.0f)), static_cast<int>(std::min(color.b * 255.0f, 255.0f)));
        }
    }
}

void scene3(Camera& c, glm::vec3 camPos, glm::mat4& V){
    vector<glm::vec3> rays;
    c.genRays(rays);

    vector<Light> lights;

    // Light 1
    Light light1;
    light1.position = glm::vec3(1.0f, 2.0f, 2.0f);
    light1.intensity = 0.5f;
    lights.push_back(light1);

    // Light 2
    Light light2;
    light2.position = glm::vec3(-1.0f, 2.0f, -1.0f);
    light2.intensity = 0.5f;
    lights.push_back(light2);

    // Green Sphere
    Material green;
    green.diff = glm::vec3(0.0f, 1.0f, 0.0f);
    green.spec = glm::vec3(1.0f, 1.0f, 0.5f);
    green.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    green.exp = 100.0f;

    Sphere sphereG(glm::vec3(-0.5, 0.0, -0.5), 1.0f, green);

    // Red Ellipsoid
    Material red;
    red.diff = glm::vec3(1.0f, 0.0f, 0.0f);
    red.spec = glm::vec3(1.0f, 1.0f, 0.5f);
    red.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    red.exp = 100.0f;

    MatrixStack modelMatrix;
    modelMatrix.translate(0.5f, 0.0f, 0.5f);
    modelMatrix.scale(0.5f, 0.6f, 0.2f);
    Ellipsoid ellipsoidR(modelMatrix.topMatrix(), red);

    // Plane
    Material white;
    white.diff = glm::vec3(1.0f, 1.0f, 1.0f);
    white.spec = glm::vec3(0.0f, 0.0f, 0.0f);
    white.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    white.exp = 0.0f;

    Plane plane(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), white);

    Scene scene;
    scene.addShape(&sphereG);
    scene.addShape(&ellipsoidR);
    scene.addShape(&plane);


    // Draw each pixel
    for (int i = 0; i < rays.size(); i++) {
        Hit hit;
        Material hitMaterial;
        bool rayHit = scene.hit(camPos, rays[i], hit, hitMaterial);
        if(rayHit)
        {
            int x = i % width;
            int y = i / width;

            if(hit.t < depthBuffer[x][y]){
                depthBuffer[x][y] = hit.t;
            }else{                
                continue;
            }


            glm::vec3 color = blinnPhongShading(hitMaterial, lights, scene, camPos, rays[i], hit, true);
            // glm::vec3 color = normalShader(hit);
            output->setPixel(x, y, static_cast<int>(std::min(color.r * 255.0f, 255.0f)), static_cast<int>(std::min(color.g * 255.0f, 255.0f)), static_cast<int>(std::min(color.b * 255.0f, 255.0f)));
        }
    }
}

void scene4and5(Camera& c, glm::vec3 camPos, glm::mat4& V){
    vector<glm::vec3> rays;
    c.genRays(rays);

    vector<Light> lights;

    // Light 1
    Light light1;
    light1.position = glm::vec3(-1.0f, 2.0f, 1.0f);
    light1.intensity = 0.5f;
    lights.push_back(light1);

    // Light 2
    Light light2;
    light2.position = glm::vec3(0.5f, -0.5f, 0.0f);
    light2.intensity = 0.5f;
    lights.push_back(light2);

    // Red Sphere
    Material red;
    red.diff = glm::vec3(1.0f, 0.0f, 0.0f);
    red.spec = glm::vec3(1.0f, 1.0f, 0.5f);
    red.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    red.exp = 100.0f;
    Sphere sphereR(glm::vec3(0.5f, -0.7f, 0.5f), 0.3f, red);

    // Blue Sphere
    Material blue;
    blue.diff = glm::vec3(0.0f, 0.0f, 1.0f);
    blue.spec = glm::vec3(1.0f, 1.0f, 0.5f);
    blue.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    blue.exp = 100.0f;
    Sphere sphereB(glm::vec3(1.0f, -0.7f, 0.0f), 0.3f, blue);

    // Floor
    Material white;
    white.diff = glm::vec3(1.0f, 1.0f, 1.0f);
    white.spec = glm::vec3(0.0f, 0.0f, 0.0f);
    white.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    white.exp = 0.0f;
    Plane floor(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), white);

    // Back Wall
    Plane backWall(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 1.0f), white);

    // Reflective
    Material reflective;
    reflective.isReflective = true;

    // Reflective Sphere 1
    Sphere sphereRef1(glm::vec3(-0.5f, 0.0f, -0.5f), 1.0f, reflective);

    // Reflective Sphere 2
    Sphere sphereRef2(glm::vec3(1.5f, 0.0f, -1.5f), 1.0f, reflective);

    Scene scene;
    scene.addShape(&sphereR);
    scene.addShape(&sphereB);
    scene.addShape(&floor);
    scene.addShape(&backWall);
    scene.addShape(&sphereRef1);
    scene.addShape(&sphereRef2);

    // Draw each pixel
    for (int i = 0; i < rays.size(); i++) {
        Hit hit;
        Material hitMaterial;
        bool rayHit = scene.hit(camPos, rays[i], hit, hitMaterial);
        if(rayHit)
        {
            int x = i % width;
            int y = i / width;

            if(hit.t < depthBuffer[x][y]){
                depthBuffer[x][y] = hit.t;
            }else{                
                continue;
            }


            glm::vec3 color = blinnPhongShading(hitMaterial, lights, scene, camPos, rays[i], hit, true);
            // glm::vec3 color = normalShader(hit);
            output->setPixel(x, y, static_cast<int>(std::min(color.r * 255.0f, 255.0f)), static_cast<int>(std::min(color.g * 255.0f, 255.0f)), static_cast<int>(std::min(color.b * 255.0f, 255.0f)));
        }
    }
}

void scene6(Camera& c, glm::vec3 camPos, glm::mat4& V){
    vector<glm::vec3> rays;
    c.genRays(rays);

    vector<Light> lights;

    // Light 1
    Light light1;
    light1.position = glm::vec3(-1.0f, 1.0f, 1.0f);
    light1.intensity = 1.0f;
    lights.push_back(light1);

    // Blue Sphere
    Material blue;
    blue.diff = glm::vec3(0.0f, 0.0f, 1.0f);
    blue.spec = glm::vec3(1.0f, 1.0f, 0.5f);
    blue.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    blue.exp = 100.0f;

    Mesh bunny("../resources/bunny.obj", glm::mat4(1.0f), blue);

    Scene scene;
    scene.addShape(&bunny);

    // Draw each pixel
    for (int i = 0; i < rays.size(); i++) {
        Hit hit;
        Material hitMaterial;
        bool rayHit = scene.hit(camPos, rays[i], hit, hitMaterial);
        if(rayHit)
        {
            int x = i % width;
            int y = i / width;

            if(hit.t < depthBuffer[x][y]){
                depthBuffer[x][y] = hit.t;
            }else{                
                continue;
            }

            glm::vec3 color = blinnPhongShading(hitMaterial, lights, scene, camPos, rays[i], hit, true);
            // glm::vec3 color = normalShader(hit);
            output->setPixel(x, y, static_cast<int>(std::min(color.r * 255.0f, 255.0f)), static_cast<int>(std::min(color.g * 255.0f, 255.0f)), static_cast<int>(std::min(color.b * 255.0f, 255.0f)));
        }
    }
}

void scene7(Camera& c, glm::vec3 camPos, glm::mat4& V){
    vector<glm::vec3> rays;
    c.genRays(rays);

    vector<Light> lights;

    // Light 1
    Light light1;
    light1.position = glm::vec3(1.0f, 1.0f, 2.0f);
    light1.intensity = 1.0f;
    lights.push_back(light1);

    // Blue Sphere
    Material blue;
    blue.diff = glm::vec3(0.0f, 0.0f, 1.0f);
    blue.spec = glm::vec3(1.0f, 1.0f, 0.5f);
    blue.amb = glm::vec3(0.1f, 0.1f, 0.1f);
    blue.exp = 100.0f;

    MatrixStack modelMatrix;
    modelMatrix.translate(0.3f, -1.5f, 0.0f);
    modelMatrix.rotate(20*M_PI/180, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix.scale(1.5f, 1.5f, 1.5f);

    Mesh bunny("../resources/bunny.obj", modelMatrix.topMatrix(), blue);

    Scene scene;
    scene.addShape(&bunny);

    // Draw each pixel
    for (int i = 0; i < rays.size(); i++) {
        Hit hit;
        Material hitMaterial;
        bool rayHit = scene.hit(camPos, rays[i], hit, hitMaterial);
        if(rayHit)
        {
            int x = i % width;
            int y = i / width;

            if(hit.t < depthBuffer[x][y]){
                depthBuffer[x][y] = hit.t;
            }else{                
                continue;
            }

            glm::vec3 color = blinnPhongShading(hitMaterial, lights, scene, camPos, rays[i], hit, true);
            // glm::vec3 color = normalShader(hit);
            output->setPixel(x, y, static_cast<int>(std::min(color.r * 255.0f, 255.0f)), static_cast<int>(std::min(color.g * 255.0f, 255.0f)), static_cast<int>(std::min(color.b * 255.0f, 255.0f)));
        }
    }
}


int main(int argc, char **argv)
{
    if (argc != 4) {
        cout << "./A6 <SCENE> <IMAGE SIZE> <IMAGE FILENAME> " << endl;
        return 1;
    }
    
    int scene = stoi(argv[1]);
    int size = stoi(argv[2]);
    string outputImage(argv[3]);
    
    width = size;
    height = size;
    float aspect = width / height;
    for(int i = 0; i < width; i++){
        depthBuffer.push_back(vector<float>());
        for(int j = 0; j < height; j++){
            depthBuffer[i].push_back(numeric_limits<float>::max());
        }
    }

    output = new Image(width, height);

    Camera camera(width, height, 45.0f, aspect, glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    if(scene == 8){
        camera = Camera(width, height, 60.0f, aspect, glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    vector<glm::vec3> rays;
    camera.genRays(rays);

	auto MV = make_shared<MatrixStack>();
	MV->loadIdentity();
	camera.applyViewMatrix(MV);
	glm::mat4 V = MV->topMatrix();
    glm::mat4 C = inverse(V);
	glm::vec3 camPos = glm::vec3(C[3][0], C[3][1], C[3][2]);

    switch (scene) {
        case 1:
            scene1(camera, camPos, V);
            break;
        case 2:
        case 8:
            scene2(camera, camPos, V);
            break;
        case 3:
            scene3(camera, camPos, V);
            break;
        case 4:
        case 5:
            scene4and5(camera, camPos, V);
            break;
        case 6:
            scene6(camera, camPos, V);
            break;
        case 7:
            scene7(camera, camPos, V);
            break;
        default:
            std::cout << "Invalid scene number: " << scene << std::endl;
            break;
    }

    output->writeToFile("./" + outputImage);
    return 0;
}

