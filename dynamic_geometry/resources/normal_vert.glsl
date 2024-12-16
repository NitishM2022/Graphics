#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 inverseTransposeMV;
uniform bool SOR;
uniform float t;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 color; // Pass to fragment shader

void main()
{
    if(SOR){        
      vec3 nor = aNor.xyz;
      vec3 dx = vec3(1, -1*sin(aPos.x + t)*cos(aPos.y), -1*sin(aPos.x + t)*sin(aPos.y));
      vec3 dtheta = vec3(0, -1*(cos(aPos.x + t)+2)*sin(aPos.y), (cos(aPos.x + t)+2)*cos(aPos.y));
      vec3 Normal = normalize(cross(dtheta, dx));
      //Normal = normalize((inverseTransposeMV * vec4(Normal, 0.0)).xyz);

      color = 0.5 * Normal + vec3(0.5, 0.5, 0.5);

      vec3 pos = vec3(aPos.x, (cos(aPos.x + t)+2)*cos(aPos.y), (cos(aPos.x + t)+2)*sin(aPos.y));
      gl_Position = P * (MV * vec4(pos, 1.0));
    }else{
        vec3 Normal = normalize(inverseTransposeMV * vec4(aNor, 0.0)).xyz;

        color = 0.5 * aNor + vec3(0.5, 0.5, 0.5);
        gl_Position = P * (MV * aPos);
	  }
}
