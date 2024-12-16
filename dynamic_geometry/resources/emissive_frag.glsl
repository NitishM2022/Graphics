uniform vec3 amb;
uniform vec3 diff;
uniform vec3 spec;
uniform float shiny;

uniform vec3 lightPos[20];
uniform vec3 lightCol[20];

uniform vec3 camPos;

varying vec3 FragPos;
varying vec3 Normal;

void main() {
    vec3 norm = normalize(Normal);

    // Ambient
    vec3 ambient = amb;
    vec3 fragColor = ambient;

    // for(int i = 0; i < 20; i++){
    //     // diffuse
    //     vec3 lightDir = normalize(lightPos[i] - FragPos);
    //     vec3 diffuse = diff * max(0.0, dot(norm, lightDir));

    //     // specular
    //     vec3 viewDir = normalize(camPos - FragPos);
    //     vec3 halfDir = normalize(lightDir + viewDir);
    //     vec3 specular = spec * pow(max(0.0, dot(halfDir, norm)), shiny);

    //     vec3 color =  lightCol[i] * (diffuse + specular);

    //     // attenuation
    //     float radius = length(lightPos[i] - FragPos);
    //     float attenuation = 1.0 / (1.0 + 0.0429 * radius + 0.9857 * radius * radius);
    
    //     fragColor += color * attenuation;
    // }

    gl_FragColor = vec4(fragColor, 1.0);
}