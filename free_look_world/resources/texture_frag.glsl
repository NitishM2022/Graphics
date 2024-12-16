uniform vec3 amb;
uniform vec3 diff;
uniform vec3 spec;
uniform float shiny;

uniform vec3 lightPos1;
uniform vec3 lightCol1;

uniform vec3 camPos;

uniform sampler2D texture0;

varying vec3 FragPos;
varying vec3 Normal;
varying vec2 vTex0;

void main() {
    vec3 norm = normalize(Normal);

    // Ambient
    vec3 ambient = amb;

    // Diffuse
    vec3 lightDir1 = normalize(lightPos1 - FragPos);
    vec3 diffuse1 = diff * max(0.0, dot(norm, lightDir1));

    // Specular
    vec3 viewDir = normalize(camPos - FragPos);
    vec3 halfDir1 = normalize(lightDir1 + viewDir);
    vec3 specular1 = spec * pow(max(0.0, dot(halfDir1, norm)), shiny);

    // Texture
    vec3 kd = texture2D(texture0, vTex0).rgb;

    vec3 color1 = (ambient + diffuse1 + specular1) * lightCol1 * kd;    
    gl_FragColor = vec4(color1, 1.0);
}