Nitish Malluru
932007196
nitishmalluru@tamu.edu
Task 7

Input
'WASD' Movement
'l' Toggle WireFrame

Shading
In ./resources/pass2_frag.glsl on line 95 change fragColor to pos, nor, ke, kd to get the individual buffers

Notes
I made a plane.obj in blender for the ground plane and the plane to view in the second pass

When I tried to build this code it kept failing since the linker ignored my libGLEW.a[2](glew.o) file since it was architecture 'x86_64', but it actually required architecture 'arm64'

To fix this I used
cmake -DCMAKE_APPLE_SILICON_PROCESSOR=arm64 ..
instead of
cmake ..

I was finally allowed me to build my code
