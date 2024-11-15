#version 330 core

in vec3 pos_world;
in vec3 normal_world;

out vec4 fragColor;

uniform float k_a;
uniform float k_d;
uniform float k_s;

uniform vec4 camera_pos;

uniform vec4 cAmbient;
uniform vec4 cDiffuse;
uniform vec4 cSpecular;
uniform float shininess;

uniform int numLights;
uniform int lightTypes[8];
uniform vec4 lightDirs[8];
uniform vec4 lightPos[8];
uniform vec4 lightColors[8];
uniform vec3 functions[8];
uniform float angles[8];
uniform float penumbras[8];

void main() {

    vec3 normal = normalize(normal_world);  // normalize normal vector for the interpolated ones

    fragColor = vec4(0.0);
    fragColor += k_a * cAmbient;  // Ambient term

    for (int i = 0; i < numLights; i++) {

        vec4 lightColor = lightColors[i];

        if (lightTypes[i] == 0) { // Directional light
            vec4 lightDir = normalize(lightDirs[i]);
            vec4 r = normalize(reflect(lightDir, vec4(normal, 0.f)));

            fragColor += k_d * cDiffuse * max(0.0, dot(normal, -vec3(lightDir))) * lightColor; // Diffusion term
            shininess == 0 ? fragColor += k_s * cSpecular * lightColor :
                    fragColor += k_s * cSpecular *
                    pow(max(0, dot(vec3(r), normalize(vec3(camera_pos) - pos_world))), shininess) * lightColor;  // specular term
        }

        else if (lightTypes[i] == 1) {  // Point light
            vec4 lightDir = normalize(vec4(pos_world, 1.0f) - lightPos[i]);
            vec4 r = normalize(reflect(lightDir, vec4(normal, 0.f)));
            float d = length(vec4(pos_world, 1.0f) - lightPos[i]);
            float att = min(1.0f, 1.0f / (functions[i][0] + functions[i][1] * d + functions[i][2] * d * d));

            fragColor += att * k_d * cDiffuse * max(0.0, dot(normal, -vec3(lightDir))) * lightColor; // Diffusion term
            shininess == 0 ? fragColor += att * k_s * cSpecular * lightColor :
                    fragColor += att * k_s * cSpecular *
                    pow(max(0, dot(vec3(r), normalize(vec3(camera_pos) - pos_world))), shininess) * lightColor;  // specular term
        }

        else if (lightTypes[i] == 2){  // spot light
            vec4 lightDir = normalize(vec4(pos_world, 1.0f) - lightPos[i]);
            vec4 r = normalize(reflect(lightDir, vec4(normal, 0.f)));
            float theta = acos(dot(lightDir, normalize(lightDirs[i])));
            float d = length(vec4(pos_world, 1.0f) - lightPos[i]);
            float att = min(1.0f, 1.0f / (functions[i][0] + functions[i][1] * d + functions[i][2] * d * d));

            if (theta <= angles[i] && theta > angles[i] - penumbras[i]) {
                att *= (1 + 2 * pow((theta - angles[i] + penumbras[i])/(penumbras[i]), 3)
                        - 3 * pow((theta - angles[i] + penumbras[i])/(penumbras[i]), 2));
            }
            else if (theta > angles[i]) att = 0;

            fragColor += att * k_d * cDiffuse * max(0.0, dot(normal, -vec3(lightDir))) * lightColor; // Diffusion term
            shininess == 0 ? fragColor += att * k_s * cSpecular * lightColor :
                    fragColor += att * k_s * cSpecular *
                    pow(max(0, dot(vec3(r), normalize(vec3(camera_pos) - pos_world))), shininess) * lightColor;  // specular term
        }
    }
}
