#version 120

attribute vec4 aPos;

uniform mat4 P;
uniform mat4 MV;

void main()
{
    gl_Position = P * (MV * aPos);
}