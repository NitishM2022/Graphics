#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 inverseTransposeMV;
uniform bool SOR;
uniform float t;

attribute vec4 aPos;
attribute vec3 aNor;

varying vec3 FragPos;
varying vec3 Normal;

void main()
{
    if(SOR){        
        vec3 nor = aNor.xyz;
        vec3 dx = vec3(1, -1*sin(aPos.x + t)*cos(aPos.y), -1*sin(aPos.x + t)*sin(aPos.y));
        vec3 dtheta = vec3(0, -1*(cos(aPos.x + t)+2)*sin(aPos.y), (cos(aPos.x + t)+2)*cos(aPos.y));
        Normal = normalize(cross(dtheta, dx));
        // Normal = normalize((inverseTransposeMV * vec4(Normal, 0.0)).xyz);

        vec3 pos = vec3(aPos.x, (cos(aPos.x + t)+2)*cos(aPos.y), (cos(aPos.x + t)+2)*sin(aPos.y));
        gl_Position = P * (MV * vec4(pos, 1.0));
    }else{
        Normal = normalize(inverseTransposeMV * vec4(aNor, 0.0)).xyz;
        
        FragPos = (MV * vec4(aPos.xyz, 1.0)).xyz;
        gl_Position = P * MV * vec4(aPos.xyz, 1.0);
    }
}
