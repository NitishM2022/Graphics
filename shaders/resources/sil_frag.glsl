#version 120

uniform vec3 camPos;

varying vec3 FragPos;
varying vec3 Normal;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir =  normalize(camPos - FragPos);

    float sil = dot(norm, viewDir);

    if(sil < 0.3)
    {
        gl_FragColor = vec4(0, 0, 0, 1.0);
    }else{
        gl_FragColor = vec4(1, 1, 1, 1.0);
    }
}
