#version 120

attribute vec3 aPos;
attribute vec3 aNor;

varying vec3 vPos;
varying vec3 vNor;

uniform mat4 MV;
uniform mat4 P;
uniform mat4 inverseTransposeMV;
uniform bool SOR;
uniform float t;

void main()
{
    if(SOR){
        vec3 nor = aNor.xyz;
        vec3 dx = vec3(1, -1*sin(aPos.x + t)*cos(aPos.y), -1*sin(aPos.x + t)*sin(aPos.y));
        vec3 dtheta = vec3(0, -1*(cos(aPos.x + t)+2)*sin(aPos.y), (cos(aPos.x + t)+2)*cos(aPos.y));
        vNor = normalize(cross(dx, dtheta));
        // vNor = normalize((inverseTransposeMV * vec4(Normal, 0.0)).xyz);

        vec3 pos = vec3(aPos.x, (cos(aPos.x + t)+2)*cos(aPos.y), (cos(aPos.x + t)+2)*sin(aPos.y));
        vPos = (MV * vec4(pos, 1.0)).xyz;

        gl_Position = P * vec4(vPos, 1.0);
    }else{
        vPos = (MV * vec4(aPos, 1.0)).xyz;
        vNor = normalize(inverseTransposeMV * vec4(aNor, 0.0)).xyz;
        gl_Position = P * vec4(vPos, 1.0);
    }
}