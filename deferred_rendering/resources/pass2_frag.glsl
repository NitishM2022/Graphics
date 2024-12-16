#version 120

uniform sampler2D posTexture;
uniform sampler2D norTexture;
uniform sampler2D keTexture;
uniform sampler2D kdTexture;
uniform vec2 windowSize;
uniform bool blur;

// uniform vec3 spec;
// uniform float shiny;
uniform vec3 lightPos[20];
uniform vec3 lightCol[20];
uniform vec3 camPos;


vec2 poissonDisk[] = vec2[](
    vec2(-0.220147, 0.976896),
    vec2(-0.735514, 0.693436),
    vec2(-0.200476, 0.310353),
    vec2( 0.180822, 0.454146),
    vec2( 0.292754, 0.937414),
    vec2( 0.564255, 0.207879),
    vec2( 0.178031, 0.024583),
    vec2( 0.613912,-0.205936),
    vec2(-0.385540,-0.070092),
    vec2( 0.962838, 0.378319),
    vec2(-0.886362, 0.032122),
    vec2(-0.466531,-0.741458),
    vec2( 0.006773,-0.574796),
    vec2(-0.739828,-0.410584),
    vec2( 0.590785,-0.697557),
    vec2(-0.081436,-0.963262),
    vec2( 1.000000,-0.100160),
    vec2( 0.622430, 0.680868)
);

vec3 sampleTextureArea(sampler2D texture, vec2 tex0)
{
    const int N = 18; // [1-18]
    const float blur = 0.005;
    vec3 val = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < N; i++) {
        val += texture2D(texture, tex0.xy + poissonDisk[i]*blur).rgb;
    }
    val /= N;
    return val;
}

void main()
{
    vec2 tex;
    tex.x = gl_FragCoord.x / windowSize.x;
    tex.y = gl_FragCoord.y / windowSize.y;

    // Fetch shading data
    vec3 pos;
    vec3 nor;
    vec3 ke;
    vec3 kd;
    if(blur){
        pos = sampleTextureArea(posTexture, tex).rgb;
        nor = sampleTextureArea(norTexture, tex).rgb;
        ke = sampleTextureArea(keTexture, tex).rgb;
        kd = sampleTextureArea(kdTexture, tex).rgb;
    }else{
        pos = texture2D(posTexture, tex).rgb;
        nor = texture2D(norTexture, tex).rgb;
        ke = texture2D(keTexture, tex).rgb;
        kd = texture2D(kdTexture, tex).rgb;
    }

    vec3 norm = normalize(nor);
    vec3 fragColor = ke;

    for (int i = 0; i < 20; i++) {
        // Diffuse
        vec3 lightDir = normalize(lightPos[i] - pos);
        vec3 diffuse = kd * max(0.0, dot(norm, lightDir));

        // Specular
        vec3 viewDir = normalize(camPos - pos);
        vec3 halfDir = normalize(lightDir + viewDir);
        vec3 specular = vec3(1.0, 1.0, 1.0) * pow(max(0.0, dot(halfDir, norm)), 10);

        vec3 color = lightCol[i] * (diffuse + specular);
    
        // Attenuation
        float radius = length(lightPos[i] - pos);
        float attenuation = 1.0 / (1.0 + 0.0429 * radius + 0.9857 * radius * radius);

        fragColor += color * attenuation;
    }

    gl_FragColor = vec4(fragColor, 1.0);
}

