#version 330 core

#define MAX_LIGHTS 16 
#define PI 3.14159

// From the vertex shader
in vec2 v_texCoord;  
in vec2 v_modelPos;

// sampler2D is a reference to a 2D image in GPU memory (SOIL stores images in GPU!)
uniform sampler2D u_texture;
uniform float u_time;

uniform int u_lightCount;
uniform vec2 u_lightPos[MAX_LIGHTS];            // Position of the light
uniform float u_lightRadius[MAX_LIGHTS];        // Radius light illuminates
uniform float u_lightIntensity[MAX_LIGHTS];     // Light intensity (0-1)
uniform vec3 u_lightColor[MAX_LIGHTS];          // Color light illuminates

// This can be any name
out vec4 FragColor;

void main() {
    const vec3 highlight = vec3(1.0,1.0,0.4); // Warm yellow
    const vec2 atlasGridSize = vec2(28.0, 20.0); 
    vec2 uv = fract(v_texCoord * atlasGridSize);
    
    vec4 baseColor = texture(u_texture, v_texCoord);

    vec3 ambient = vec3(0.10); // Add an ambient light (can be changed)
    vec3 lightAccum = ambient;

    for (int i = 0; i < u_lightCount; i++) {
        float dist = distance(v_modelPos, u_lightPos[i]); // Distance from pixel to lightPos
        float brightness = u_lightIntensity[i] - clamp(dist / u_lightRadius[i], 0.0, u_lightIntensity[i]); // Force X to stay within min, max value
        brightness = brightness * brightness; // Soften
        vec3 light = u_lightColor[i] * brightness * u_lightIntensity[i];

        lightAccum += light;
    }

    lightAccum = clamp(lightAccum, 0.0, 1.0);

    baseColor.rgb *= lightAccum;

    float shimmer_intensity = 0.25;
    shimmer_intensity *= 0.5;
    shimmer_intensity = clamp(shimmer_intensity, 0.0, 1.0); 

    float shimmer_brightness = shimmer_intensity* sin(2 * PI * u_time) + (1 - shimmer_intensity);
    baseColor.rgb *= shimmer_brightness; // Applies a shimmer effect

    FragColor = baseColor;
}