#version 120

varying vec3 vPos;
varying vec3 vNor;

uniform vec3 amb;
uniform vec3 diff;

void main()
{
    gl_FragData[0].xyz = vPos;
    gl_FragData[1].xyz = vNor;
    gl_FragData[2].xyz = amb;
    gl_FragData[3].xyz = diff;
}