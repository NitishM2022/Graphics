#include <iostream>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Image.h"
#include <cmath>

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

double RANDOM_COLORS[7][3] = {
    {0.0000,    0.4470,    0.7410},
    {0.8500,    0.3250,    0.0980},
    {0.9290,    0.6940,    0.1250},
    {0.4940,    0.1840,    0.5560},
    {0.4660,    0.6740,    0.1880},
    {0.3010,    0.7450,    0.9330},
    {0.6350,    0.0780,    0.1840},
};

double width;
double height;

double xmin = INT_MAX;
double xmax = 0;

double ymin = INT_MAX;
double ymax = 0;

double zmin = INT_MAX;
double zmax = 0;

Image* output;

//task 1
double calcCoord(bool x, bool z, double coord);
void drawBoundingBoxes(vector<float>& posBuf);

//task 2
bool inTriangle(int x, int y, double px0, double py0, double px1, double py1,  double px2, double py2);
double calcInnerArea(double x, double y, double px0, double py0, double px1, double py1);
void drawTriangles(vector<float>& posBuf, vector<std::vector<double>>& zbuff);

//task 3
void calcABC(int x, int y, double px0, double py0, double px1, double py1,  double px2, double py2, double& a, double& b, double& c);
void interpolateTriangle(vector<float>& posBuf, vector<std::vector<double>>& zbuff);

//task 4
double calcPercentHeight(double ymax, double ymin, int coord);
void interpolateVertical(vector<float>& posBuf);

//task 5
void interpolateDepth(vector<float>& posBuf, vector<std::vector<double>>& zbuff);

//task 6
void colorNormal(vector<float>& posBuf, vector<float>& norBuf, vector<std::vector<double>>& zbuff);

//task 7
void basicLighting(vector<float>& posBuf, vector<float>& norBuf, vector<std::vector<double>>& zbuff);

//task 8
void rotateYAxis(double angle, double& x, double& y, double& z);



int main(int argc, char **argv)
{
    if (argc != 6) {
        cout << "Usage: A1 meshFile outputImageFile width height taskNumber" << endl;
        return 1;
    }
    
    string meshName(argv[1]);
    string outputImage(argv[2]);
    width = stoi(argv[3]);
    height = stoi(argv[4]);
    int taskNumber = stoi(argv[5]);
    
    
    output = new Image(width, height);
    vector<vector<double>> zbuff;// current z at set pixel
    for(int x = 0; x < width; x++){
        zbuff.push_back(vector<double>());
        for(int y = 0; y < height; y++){
            zbuff[x].push_back(-1); // indicates value is unitialized
        }
    }
    
    // Load geometry
    vector<float> posBuf; // list of vertex positions
    vector<float> norBuf; // list of vertex normals
    vector<float> texBuf; // list of vertex texture coords
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
                    
                    double angle = M_PI/4;
                    
                    if(taskNumber == 8){
                        double x = attrib.vertices[3*idx.vertex_index+0];
                        double y = attrib.vertices[3*idx.vertex_index+1];
                        double z = attrib.vertices[3*idx.vertex_index+2];
                        
                        rotateYAxis(angle, x, y, z);
                        
                        posBuf.push_back(x);
                        posBuf.push_back(y);
                        posBuf.push_back(z);
                        
                        // find the max and min vertices of the x and y coordinate
                        // this will help when trying to fit the image
                        xmin = min(xmin, x);
                        xmax = max(xmax, x);
                        
                        ymin = min(ymin, y);
                        ymax = max(ymax, y);
                        
                        zmin = min(zmin, z);
                        zmax = max(zmax, z);
                    }else{
                        posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
                        posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
                        posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);
                        
                        // find the max and min vertices of the x and y coordinate
                        // this will help when trying to fit the image
                        xmin = min(xmin, (double) attrib.vertices[3*idx.vertex_index+0]);
                        xmax = max(xmax, (double) attrib.vertices[3*idx.vertex_index+0]);
                        
                        ymin = min(ymin, (double) attrib.vertices[3*idx.vertex_index+1]);
                        ymax = max(ymax, (double) attrib.vertices[3*idx.vertex_index+1]);
                        
                        zmin = min(zmin, (double) attrib.vertices[3*idx.vertex_index+2]);
                        zmax = max(zmax, (double) attrib.vertices[3*idx.vertex_index+2]);
                    }
                    
                    if(!attrib.normals.empty()) {
                        if(taskNumber == 8){
                            double x = attrib.normals[3*idx.normal_index+0];
                            double y = attrib.normals[3*idx.normal_index+1];
                            double z = attrib.normals[3*idx.normal_index+2];
                            
                            rotateYAxis(angle, x, y, z);
                            
                            norBuf.push_back(x);
                            norBuf.push_back(y);
                            norBuf.push_back(z);
                        }else{
                            norBuf.push_back(attrib.normals[3*idx.normal_index+0]);
                            norBuf.push_back(attrib.normals[3*idx.normal_index+1]);
                            norBuf.push_back(attrib.normals[3*idx.normal_index+2]);
                        }
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
    cout << "Number of vertices: " << posBuf.size()/3 << endl;
    
    
    if(taskNumber == 1){
        drawBoundingBoxes(posBuf);
    }else if(taskNumber == 2){
        drawTriangles(posBuf, zbuff);
    }else if(taskNumber == 3){
        interpolateTriangle(posBuf, zbuff);
    }else if(taskNumber == 4){
        interpolateVertical(posBuf);
    }else if(taskNumber == 5){
        interpolateDepth(posBuf, zbuff);
    }else if(taskNumber == 6){
        colorNormal(posBuf, norBuf, zbuff);
    }else if(taskNumber == 7 || taskNumber == 8){
        basicLighting(posBuf, norBuf, zbuff);
    }
    
    output->writeToFile("./" + outputImage);
    return 0;
}

//have to think about if the x1-x0 is bigger than width
double calcCoord(bool x, bool z, double coord){
    
    //ensure that any vertices are not negative
    double pxmax = xmax;
    double pxmin = xmin;
    
    double pymax = ymax;
    double pymin = ymin;
    
    if(xmin < 0){
        pxmax = xmax - xmin;
        if(x){
            coord -= xmin;
        }
        pxmin = 0;
    }
    
    if(ymin < 0){
        pymax = ymax - ymin;
        if(!x){
            coord -= ymin;
        }
        pymin = 0;
    }
    
    if(zmin < 0 && z){
        coord -= zmin;
    }
    
    double occupyx = (pxmax-pxmin)/ width;
    double occupyy = (pymax-pymin)/ height;
    if(occupyx >= occupyy){
        double mid = (pymax-pymin)/2.0;
        double convx = width / (pxmax - pxmin);
        
        if(z || x){
            return (convx-pxmin) * coord;
        }else{ // y
            double offset = height/2 - convx * mid;
            return convx * (coord - pymin) + offset; // must subtract min if y dimension potentially has a bigger max to start it at 0
        }
        
    }else{
        double mid = (pxmax-pxmin)/2.0;
        double convy = height / (pymax - pymin);
        
        if(z || !x){
            return convy * (coord - pymin);
        }
        else{ // x
            double offset = width/2 - convy * mid;
            return convy * (coord-pxmin) + offset;
        }
    }
}



void drawBoundingBoxes(vector<float>& posBuf){
    //each iteration represents one triangle
    for(int i = 0; i <= posBuf.size() - 9; i+=9){
        double r = RANDOM_COLORS[(i/9)%7][0] * 256;
        double g = RANDOM_COLORS[(i/9)%7][1] * 256;
        double b = RANDOM_COLORS[(i/9)%7][2] * 256;
        
        // get all the triangle points
        double px0 = calcCoord(true, false, posBuf[i]);
        double py0 = calcCoord(false, false, posBuf[i+1]);
        
        double px1 = calcCoord(true, false, posBuf[i+3]);
        double py1 = calcCoord(false, false, posBuf[i+4]);
        
        double px2 = calcCoord(true, false, posBuf[i+6]);
        double py2 = calcCoord(false, false, posBuf[i+7]);
        
        //draw a bounding box around triangle
        double xpoint0 = min(px0, min(px1, px2));
        double ypoint0 = min(py0, min(py1, py2));
        
        double xpoint1 = max(px0, max(px1, px2));
        double ypoint1 = max(py0, max(py1, py2));
        
        for(int x = floor(xpoint0); x < ceil(xpoint1); x++){
            for(int y = floor(ypoint0); y < ceil(ypoint1); y++){
                output->setPixel(x, y, r, g, b);
            }
        }
        
    }
}

bool inTriangle(int x, int y, double px0, double py0, double px1, double py1,  double px2, double py2){
    double totalArea = (calcInnerArea(px0, py0, px1, py1, px2, py2));
    double a = calcInnerArea(x, y, px1, py1, px2, py2)/totalArea;
    double b = calcInnerArea(x, y, px2, py2, px0, py0)/totalArea;
    double c = calcInnerArea(x, y, px0, py0, px1, py1)/totalArea;
    
    if(a < 0 ){
        return false;
    }
    
    if(b < 0){
        return false;
    }
    
    if(c < 0){
        return false;
    }
    
    if(a+b+c > 1.001){
        return false;
    }
    
    return true;
}



double calcInnerArea(double x, double y, double px0, double py0, double px1, double py1){
    return 0.5 * ((px0-x)*(py1-y)-(px1-x)*(py0-y));
}

void drawTriangles(vector<float>& posBuf, vector<std::vector<double>>& zbuff){
    //each iteration represents one triangle
    for(int i = 0; i <= posBuf.size() - 9; i+=9){
        double r = RANDOM_COLORS[(i/9)%7][0] * 256;
        double g = RANDOM_COLORS[(i/9)%7][1] * 256;
        double b = RANDOM_COLORS[(i/9)%7][2] * 256;
        
        // get all the triangle points
        double px0 = calcCoord(true, false, posBuf[i]);
        double py0 = calcCoord(false, false,posBuf[i+1]);
        double pz0 = calcCoord(false, true, posBuf[i+2]);
        
        double px1 = calcCoord(true, false,posBuf[i+3]);
        double py1 = calcCoord(false, false, posBuf[i+4]);
        double pz1 = calcCoord(false, true, posBuf[i+5]);
        
        double px2 = calcCoord(true, false,posBuf[i+6]);
        double py2 = calcCoord(false, false,posBuf[i+7]);
        double pz2 = calcCoord(false, true, posBuf[i+8]);
        
        //draw a bounding box around triangle
        double xpoint0 = min(px0, min(px1, px2));
        double ypoint0 = min(py0, min(py1, py2));
        
        double xpoint1 = max(px0, max(px1, px2));
        double ypoint1 = max(py0, max(py1, py2));
        
        for(int x = floor(xpoint0); x < ceil(xpoint1); x++){
            for(int y = floor(ypoint0); y < ceil(ypoint1); y++){
                if(inTriangle(x, y, px0, py0, px1, py1, px2, py2)){
                    double aVal;
                    double bVal;
                    double cVal;
                    
                    calcABC(x, y, px0, py0, px1, py1, px2, py2, aVal, bVal, cVal);
                    double zval = aVal*pz0 + bVal*pz1 + cVal*pz2;
                    if(zval > zbuff[x][y]){
                        zbuff[x][y] = zval;
                        output->setPixel(x, y, r, g, b);
                    }
                }
            }
        }
    }
}

void calcABC(int x, int y, double px0, double py0, double px1, double py1,  double px2, double py2, double& a, double& b, double& c){
    double totalArea = calcInnerArea(px0, py0, px1, py1, px2, py2);
    a = calcInnerArea(x, y, px1, py1, px2, py2)/totalArea;
    b = calcInnerArea(x, y, px2, py2, px0, py0)/totalArea;
    c = calcInnerArea(x, y, px0, py0, px1, py1)/totalArea;
}


void interpolateTriangle(vector<float>& posBuf, vector<std::vector<double>>& zbuff){
    //each iteration represents one triangle
    for(int i = 0; i <= posBuf.size() - 9; i+=9){
        double r0 = RANDOM_COLORS[(i/9+0)%7][0] * 256;
        double r1 = RANDOM_COLORS[(i/9+1)%7][0] * 256;
        double r2 = RANDOM_COLORS[(i/9+2)%7][0] * 256;
        
        double g0 = RANDOM_COLORS[(i/9+0)%7][1] * 256;
        double g1 = RANDOM_COLORS[(i/9+1)%7][1] * 256;
        double g2 = RANDOM_COLORS[(i/9+2)%7][1] * 256;
        
        double b0 = RANDOM_COLORS[(i/9+0)%7][2] * 256;
        double b1 = RANDOM_COLORS[(i/9+1)%7][2] * 256;
        double b2 = RANDOM_COLORS[(i/9+2)%7][2] * 256;
        
        // get all the triangle points
        double px0 = calcCoord(true, false, posBuf[i]);
        double py0 = calcCoord(false, false,posBuf[i+1]);
        double pz0 = calcCoord(false, true, posBuf[i+2]);
        
        double px1 = calcCoord(true, false,posBuf[i+3]);
        double py1 = calcCoord(false, false, posBuf[i+4]);
        double pz1 = calcCoord(false, true, posBuf[i+5]);
        
        double px2 = calcCoord(true, false,posBuf[i+6]);
        double py2 = calcCoord(false, false,posBuf[i+7]);
        double pz2 = calcCoord(false, true, posBuf[i+8]);
        
        //draw a bounding box around triangle
        double xpoint0 = min(px0, min(px1, px2));
        double ypoint0 = min(py0, min(py1, py2));
        
        double xpoint1 = max(px0, max(px1, px2));
        double ypoint1 = max(py0, max(py1, py2));
        
        for(int x = floor(xpoint0); x < ceil(xpoint1); x++){
            for(int y = floor(ypoint0); y < ceil(ypoint1); y++){
                
                if(inTriangle(x, y, px0, py0, px1, py1, px2, py2)){
                    double aVal;
                    double bVal;
                    double cVal;
                    
                    calcABC(x, y, px0, py0, px1, py1, px2, py2, aVal, bVal, cVal);
                    double zval = aVal*pz0 + bVal*pz1 + cVal*pz2;
                    if(zval > zbuff[x][y]){
                        zbuff[x][y] = zval;
                        output->setPixel(x, y, r0*aVal+r1*bVal+r2*cVal, g0*aVal+g1*bVal+g2*cVal, b0*aVal+b1*bVal+b2*cVal);
                    }
                }
            }
        }
    }
}

double calcPercentHeight(double ymax, double ymin, int coord){
    return (coord-ymin)/(ymax - ymin);
}


void interpolateVertical(vector<float>& posBuf){
    //each iteration represents one triangle
    for(int i = 0; i <= posBuf.size() - 9; i+=9){
        // get all the triangle points
        double px0 = calcCoord(true, false, posBuf[i]);
        double py0 = calcCoord(false, false,posBuf[i+1]);
        
        double px1 = calcCoord(true, false,posBuf[i+3]);
        double py1 = calcCoord(false, false, posBuf[i+4]);
        
        double px2 = calcCoord(true, false,posBuf[i+6]);
        double py2 = calcCoord(false, false,posBuf[i+7]);
        
        //draw a bounding box around triangle
        double xpoint0 = min(px0, min(px1, px2));
        double ypoint0 = min(py0, min(py1, py2));
        
        double xpoint1 = max(px0, max(px1, px2));
        double ypoint1 = max(py0, max(py1, py2));
        
        for(int x = floor(xpoint0); x < ceil(xpoint1); x++){
            for(int y = floor(ypoint0); y < ceil(ypoint1); y++){
                if(inTriangle(x, y, px0, py0, px1, py1, px2, py2)){
                    double red = calcPercentHeight(calcCoord(false, false, ymax), calcCoord(false, false, ymin), y);
                    output->setPixel(x, y, 255*red, 0, 255*(1-red));
                }
            }
        }
    }
}


void interpolateDepth(vector<float>& posBuf, vector<std::vector<double>>& zbuff){
    //each iteration represents one triangle
    for(int i = 0; i <= posBuf.size() - 9; i+=9){
        // get all the triangle points
        double px0 = calcCoord(true, false, posBuf[i]);
        double py0 = calcCoord(false, false,posBuf[i+1]);
        double pz0 = calcCoord(false, true, posBuf[i+2]);
        
        double px1 = calcCoord(true, false,posBuf[i+3]);
        double py1 = calcCoord(false, false, posBuf[i+4]);
        double pz1 = calcCoord(false, true, posBuf[i+5]);
        
        double px2 = calcCoord(true, false,posBuf[i+6]);
        double py2 = calcCoord(false, false,posBuf[i+7]);
        double pz2 = calcCoord(false, true, posBuf[i+8]);
        
        //draw a bounding box around triangle
        double xpoint0 = min(px0, min(px1, px2));
        double ypoint0 = min(py0, min(py1, py2));
        
        double xpoint1 = max(px0, max(px1, px2));
        double ypoint1 = max(py0, max(py1, py2));
        
        for(int x = floor(xpoint0); x < ceil(xpoint1); x++){
            for(int y = floor(ypoint0); y < ceil(ypoint1); y++){
                if(inTriangle(x, y, px0, py0, px1, py1, px2, py2)){
                    double aVal;
                    double bVal;
                    double cVal;
                    
                    calcABC(x, y, px0, py0, px1, py1, px2, py2, aVal, bVal, cVal);
                    double zval = aVal*pz0 + bVal*pz1 + cVal*pz2;
                    if(zval > zbuff[x][y]){
                        zbuff[x][y] = zval;
                    }
                }
            }
        }
    }
    
    //color based on depth now
    double calczmin = calcCoord(false, true, zmin);
    double calczmax = calcCoord(false, true, zmax);
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            if(zbuff[x][y] > -1){
                double red = (zbuff[x][y] - calczmin)/(calczmax - calczmin);
                output->setPixel(x, y, 255*red, 0, 0);
            }
        }
    }
}

void colorNormal(vector<float>& posBuf, vector<float>& norBuf, vector<std::vector<double>>& zbuff){
    //each iteration represents one triangle
    for(int i = 0; i <= posBuf.size() - 9; i+=9){
        double r0 = 255 * (0.5 * norBuf[i] + 0.5);
        double r1 = 255 * (0.5 * norBuf[i+3] + 0.5);
        double r2 = 255 * (0.5 * norBuf[i+6] + 0.5);
        
        double g0 = 255 * (0.5 * norBuf[i+1] + 0.5);
        double g1 = 255 * (0.5 * norBuf[i+4] + 0.5);
        double g2 = 255 * (0.5 * norBuf[i+7] + 0.5);
        
        double b0 = 255 * (0.5 * norBuf[i+2] + 0.5);
        double b1 = 255 * (0.5 * norBuf[i+5] + 0.5);
        double b2 = 255 * (0.5 * norBuf[i+8] + 0.5);
        
        // get all the triangle points
        double px0 = calcCoord(true, false, posBuf[i]);
        double py0 = calcCoord(false, false,posBuf[i+1]);
        double pz0 = calcCoord(false, true, posBuf[i+2]);
        
        double px1 = calcCoord(true, false,posBuf[i+3]);
        double py1 = calcCoord(false, false, posBuf[i+4]);
        double pz1 = calcCoord(false, true, posBuf[i+5]);
        
        double px2 = calcCoord(true, false,posBuf[i+6]);
        double py2 = calcCoord(false, false,posBuf[i+7]);
        double pz2 = calcCoord(false, true, posBuf[i+8]);
        
        //draw a bounding box around triangle
        double xpoint0 = min(px0, min(px1, px2));
        double ypoint0 = min(py0, min(py1, py2));
        
        double xpoint1 = max(px0, max(px1, px2));
        double ypoint1 = max(py0, max(py1, py2));
        
        for(int x = floor(xpoint0); x < ceil(xpoint1); x++){
            for(int y = floor(ypoint0); y < ceil(ypoint1); y++){
                if(inTriangle(x, y, px0, py0, px1, py1, px2, py2)){
                    double aVal;
                    double bVal;
                    double cVal;
                    
                    calcABC(x, y, px0, py0, px1, py1, px2, py2, aVal, bVal, cVal);
                    double zval = aVal*pz0 + bVal*pz1 + cVal*pz2;
                    if(zval > zbuff[x][y]){
                        zbuff[x][y] = zval;
                        output->setPixel(x, y, r0*aVal+r1*bVal+r2*cVal, g0*aVal+g1*bVal+g2*cVal, b0*aVal+b1*bVal+b2*cVal);
                    }
                }
            }
        }
    }
}

void basicLighting(vector<float>& posBuf, vector<float>& norBuf, vector<std::vector<double>>& zbuff){
    //each iteration represents one triangle
    for(int i = 0; i <= posBuf.size() - 9; i+=9){
        double c0 = (1/sqrt(3))*norBuf[i] + (1/sqrt(3))*norBuf[i+1] + (1/sqrt(3))*norBuf[i+2];
        double c1 = (1/sqrt(3))*norBuf[i+3] + (1/sqrt(3))*norBuf[i+4] + (1/sqrt(3))*norBuf[i+5];
        double c2 = (1/sqrt(3))*norBuf[i+6] + (1/sqrt(3))*norBuf[i+7] + (1/sqrt(3))*norBuf[i+8];
        // get all the triangle points
        double px0 = calcCoord(true, false, posBuf[i]);
        double py0 = calcCoord(false, false,posBuf[i+1]);
        double pz0 = calcCoord(false, true, posBuf[i+2]);
        
        double px1 = calcCoord(true, false,posBuf[i+3]);
        double py1 = calcCoord(false, false, posBuf[i+4]);
        double pz1 = calcCoord(false, true, posBuf[i+5]);
        
        double px2 = calcCoord(true, false,posBuf[i+6]);
        double py2 = calcCoord(false, false,posBuf[i+7]);
        double pz2 = calcCoord(false, true, posBuf[i+8]);
        
        //draw a bounding box around triangle
        double xpoint0 = min(px0, min(px1, px2));
        double ypoint0 = min(py0, min(py1, py2));
        
        double xpoint1 = max(px0, max(px1, px2));
        double ypoint1 = max(py0, max(py1, py2));
        
        for(int x = floor(xpoint0); x < ceil(xpoint1); x++){
            for(int y = floor(ypoint0); y < ceil(ypoint1); y++){
                if(inTriangle(x, y, px0, py0, px1, py1, px2, py2)){
                    double aVal;
                    double bVal;
                    double cVal;
                    
                    calcABC(x, y, px0, py0, px1, py1, px2, py2, aVal, bVal, cVal);
                    double zval = aVal*pz0 + bVal*pz1 + cVal*pz2;
                    if(zval > zbuff[x][y]){
                        zbuff[x][y] = zval;
                        double interpolatedLight = max(c0*aVal+c1*bVal+c2*cVal,0.0);
                        output->setPixel(x, y, 255 * interpolatedLight, 255 * interpolatedLight, 255 * interpolatedLight);
                    }
                }
            }
        }
    }
}

void rotateYAxis(double angle, double& x, double& y, double& z){
    double newX;
    double newZ;
    newX = cos(angle)*x + sin(angle)*z;
    newZ = -1*sin(angle)*x + cos(angle)*z;
    x = newX;
    z = newZ;
}
