Nitish Malluru
932007196
nitishmalluru@tamu.edu
Task 6

Input
'WASD' Movement
'z'/'Z' Zoom In and Zoom Out
't' Toggle MiniMap
'y' Shade Normals
'l' Toggle Top Down View

Notes
My texture was taken from Lab 9, and I made a plane.obj in blender for the ground plane

When I tried to build this code it kept failing since the linker ignored my libGLEW.a[2](glew.o) file since it was architecture 'x86_64', but it actually required architecture 'arm64'

To fix this I used
cmake -DCMAKE_APPLE_SILICON_PROCESSOR=arm64 ..
instead of
cmake ..

I was finally allowed me to build my code
