uniform vec3 amb;
uniform vec3 diff;
uniform vec3 spec;
uniform float shiny;

uniform vec3 lightPos1;
uniform vec3 lightCol1;
uniform vec3 lightPos2;
uniform vec3 lightCol2;


uniform vec3 camPos;

varying vec3 FragPos;
varying vec3 Normal;

void main() {
    vec3 norm = normalize(Normal);

    // Ambient
    vec3 ambient = amb;

    // Diffuse
    vec3 lightDir1 = normalize(lightPos1 - FragPos);
    vec3 diffuse1 = diff * max(0.0, dot(norm, lightDir1));

    vec3 lightDir2 = normalize(lightPos2 - FragPos);
    vec3 diffuse2 = diff * max(0.0, dot(norm, lightDir2));

    // Specular
    vec3 viewDir = normalize(camPos - FragPos);
    vec3 halfDir1 = normalize(lightDir1 + viewDir);
    vec3 specular1 = spec * pow(max(0.0, dot(halfDir1, norm)), shiny);

    vec3 halfDir2 = normalize(lightDir2 + viewDir);
    vec3 specular2 = spec * pow(max(0.0, dot(halfDir2, norm)), shiny);

    vec3 color1 = (ambient + diffuse1 + specular1)*lightCol1;
    vec3 color2 = (ambient + diffuse2 + specular2)*lightCol2;
    
    vec4 finalColor = vec4(color1+color2, 1.0);

    if(finalColor.r < 0.25) {
        finalColor.r = 0.0;
    } else if(finalColor.r < 0.5) {
        finalColor.r = 0.25;
    } else if(finalColor.r < 0.75) {
        finalColor.r = 0.5;
    } else if(finalColor.r < 1.0) {
        finalColor.r = 0.75;
    } else {
        finalColor.r = 1.0;
    }

    if(finalColor.g < 0.25) {
        finalColor.g = 0.0;
    } else if(finalColor.g < 0.5) {
        finalColor.g = 0.25;
    } else if(finalColor.g < 0.75) {
        finalColor.g = 0.5;
    } else if(finalColor.g < 1.0) {
        finalColor.g = 0.75;
    } else {
        finalColor.g = 1.0;
    }

    if(finalColor.b < 0.25) {
        finalColor.b = 0.0;
    } else if(finalColor.b < 0.5) {
        finalColor.b = 0.25;
    } else if(finalColor.b < 0.75) {
        finalColor.b = 0.5;
    } else if(finalColor.b < 1.0) {
        finalColor.b = 0.75;
    } else {
        finalColor.b = 1.0;
    }

    gl_FragColor = finalColor;
}