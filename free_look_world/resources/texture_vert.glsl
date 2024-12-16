#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 inverseTransposeMV;

attribute vec4 aPos;
attribute vec3 aNor;
attribute vec2 aTex;

varying vec3 FragPos;
varying vec3 Normal;
varying vec2 vTex0;

void main()
{
    Normal = normalize(inverseTransposeMV * vec4(aNor, 0.0)).xyz;
    
    FragPos = (MV * vec4(aPos.xyz, 1.0)).xyz;

    vTex0 = aTex;

    gl_Position = P * MV * aPos, 1.0;
}