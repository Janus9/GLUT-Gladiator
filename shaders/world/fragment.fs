#version 330 core

#define MAX_LIGHTS 16 

// From the vertex shader
in vec2 v_atlasUV;  
flat in float v_selected;
in vec2 v_modelPos;

// sampler2D is a reference to a 2D image in GPU memory (SOIL stores images in GPU!)
uniform sampler2D u_texture;
uniform vec2 u_cameraPos;
uniform float u_time;

uniform int u_lightCount;
uniform vec2 u_lightPos;            // Position of the light
uniform float u_lightRadius;        // Radius light illuminates
uniform float u_lightIntensity;     // Light intensity (0-1)
uniform vec3 u_lightColor;          // Color light illuminates

// This can be any name
out vec4 FragColor;

void main() {
    const vec3 highlight = vec3(1.0,1.0,0.4); // Warm yellow
    const vec2 atlasGridSize = vec2(28.0, 20.0); 
    vec2 uv = fract(v_atlasUV * atlasGridSize);
    
    vec4 baseColor = texture(u_texture, v_atlasUV);

    float dist = distance(v_modelPos, u_lightPos); // Distance from pixel to lightPos
    float brightness = u_lightIntensity - clamp(dist / u_lightRadius, 0.0, u_lightIntensity); // Force X to stay within min, max value
    brightness = brightness * brightness; // Soften

    vec3 ambient = vec3(0.05); // Add an ambient light (can be changed)
    vec3 light = ambient + (u_lightColor * brightness * u_lightIntensity);
    light = clamp(light, 0.0, 1.0);

    baseColor.rgb *= light;

    if (v_selected > 0.0) {
        // Selected
        // baseColor.rgb = mix(baseColor.rgb, highlight, 0.35);
        
        // check for border 
        const float outlinePct = 0.1;
        if ((uv.x > 0.0 && uv.x < outlinePct) ||          // Left Border
            (uv.x > 1.0 - outlinePct && uv.x < 1.0) ||    // Right Border
            (uv.y > 0.0 && uv.y < outlinePct) ||          // Top Border
            (uv.y > 1.0 - outlinePct && uv.y < 1.0))      // Bottom Border
        {
            baseColor.rgb = vec3(0.0,0.0,0.0);
        }
    } 

    FragColor = baseColor;
}