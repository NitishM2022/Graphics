#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <cfloat>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "common.h"
#include "raytri.h"

using namespace std;

class Mesh : public Shape 
{
public:

    Mesh(string meshName, glm::mat4 modelMatrix, Material color) 
        : meshName(meshName), 
          modelMatrix(modelMatrix),
          color(color),
          boundingEllipsoid(initGeometry()) 
    {
    }

    bool intersect(glm::vec3 origin, glm::vec3 ray, Hit& closestHit) override {
        Hit checkHit;
        glm::vec3 modelOrigin = glm::vec3(invModelMatrix * glm::vec4(origin, 1.0f));
        glm::vec3 modelRay = glm::normalize(glm::vec3(invModelMatrix * glm::vec4(ray, 0.0f)));
        if (!boundingEllipsoid.intersect(modelOrigin, modelRay, checkHit)) {
            return false; 
        }

        for (int i = 0; i < posBuf.size(); i += 9) {
            double v0[3] = {static_cast<double>(posBuf[i]), static_cast<double>(posBuf[i + 1]), static_cast<double>(posBuf[i + 2])};
            double v1[3] = {static_cast<double>(posBuf[i + 3]), static_cast<double>(posBuf[i + 4]), static_cast<double>(posBuf[i + 5])};
            double v2[3] = {static_cast<double>(posBuf[i + 6]), static_cast<double>(posBuf[i + 7]), static_cast<double>(posBuf[i + 8])};

            double originDouble[3] = {static_cast<double>(modelOrigin.x), static_cast<double>(modelOrigin.y), static_cast<double>(modelOrigin.z)};
            double rayDouble[3] = {static_cast<double>(modelRay.x), static_cast<double>(modelRay.y), static_cast<double>(modelRay.z)};
            double t, u, v;

            if (intersect_triangle2(originDouble, rayDouble, v0, v1, v2, &t, &u, &v) == 1) {
                if (t > 0.0f) {
                    glm::vec3 hitPos =  glm::vec3(modelMatrix *  glm::vec4((modelOrigin + static_cast<float>(t) * modelRay),1.0f));

                    glm::vec3 normal1 = glm::vec3(norBuf[i], norBuf[i + 1], norBuf[i + 2]); 
                    glm::vec3 normal2 = glm::vec3(norBuf[i + 3], norBuf[i + 4], norBuf[i + 5]); 
                    glm::vec3 normal3 = glm::vec3(norBuf[i + 6], norBuf[i + 7], norBuf[i + 8]);
                    glm::vec3 normal = static_cast<float>(1.0f - u - v) * normal1 + static_cast<float>(u) * normal2 + static_cast<float>(v) * normal3;
                    normal =  glm::normalize(glm::vec3(glm::transpose(invModelMatrix) * glm::vec4(normal,1.0f)));

                    float distance = glm::length(hitPos - origin);

                    if(dot(ray, (hitPos - origin)) < 0.0f){
                        continue;
                    }

                    if(closestHit.valid == false){
                        closestHit = Hit(hitPos, normal, distance);
                        closestHit.valid = true; //redundant
                    }else if (distance < closestHit.t) {
                        closestHit = Hit(hitPos, normal, distance);
                    }
                }
            }
        }

        return closestHit.valid;
    }

    Material getColor() override {
        return color;
    }

    int loadGeometry(){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        string errStr;
        bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
        if(!rc) {
            cerr << errStr << endl;
        } else {
            // Some OBJ files have different indices for vertex positions, normals,
            // and texture coordinates. For example, a cube corner vertex may have
            // three different normals. Here, we are going to duplicate all such
            // vertices.
            // Loop over shapes
            for(size_t s = 0; s < shapes.size(); s++) {
                // Loop over faces (polygons)
                size_t index_offset = 0;
                for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                    size_t fv = shapes[s].mesh.num_face_vertices[f];
                    // Loop over vertices in the face.
                    for(size_t v = 0; v < fv; v++) {
                        // access to vertex
                        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                                                
                        posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
                        posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
                        posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);

                        xmin = min(xmin, posBuf[posBuf.size()-3]);
                        xmax = max(xmax, posBuf[posBuf.size()-3]);
                        
                        ymin = min(ymin, posBuf[posBuf.size()-2]);
                        ymax = max(ymax, posBuf[posBuf.size()-2]);
                        
                        zmin = min(zmin, posBuf[posBuf.size()-1]);
                        zmax = max(zmax, posBuf[posBuf.size()-1]);

                        if(!attrib.normals.empty()) {
                            norBuf.push_back(attrib.normals[3*idx.normal_index+0]);
                            norBuf.push_back(attrib.normals[3*idx.normal_index+1]);
                            norBuf.push_back(attrib.normals[3*idx.normal_index+2]);
                        }
                        if(!attrib.texcoords.empty()) {
                            texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+0]);
                            texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+1]);
                        }
                    }
                    index_offset += fv;
                    // per-face material (IGNORE)
                    shapes[s].mesh.material_ids[f];
                }
            }
        }

        return posBuf.size()/3;
    }


private:
    string meshName;
    vector<float> posBuf;
    vector<float> norBuf;
    vector<float> texBuf;

    glm::mat4 modelMatrix;
    glm::mat4 invModelMatrix;

    float xmin = FLT_MAX;
    float xmax = -FLT_MAX;

    float ymin = FLT_MAX;
    float ymax = -FLT_MAX;

    float zmin = FLT_MAX;
    float zmax = -FLT_MAX;

    Material color;
    Ellipsoid boundingEllipsoid;

    Ellipsoid initGeometry() {
        loadGeometry();
        invModelMatrix = glm::inverse(modelMatrix);
        MatrixStack modelMatrix;
        modelMatrix.translate((xmin+xmax)/2, (ymin+ymax)/2, (zmin+zmax)/2);
        modelMatrix.scale(xmax-xmin, ymax-ymin, zmax-zmin);
        return Ellipsoid(modelMatrix.topMatrix(), color);
    }
};


#endif