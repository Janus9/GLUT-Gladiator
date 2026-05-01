#version 330 core

/*
in - Per vertex data comming from the VBO
Unform - Data for the whole draw
out - Output from vertex shader for fragment shader (usually texture coords)
*/

// Data from the VBO //
layout(location = 0) in vec2 a_localPos;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec2 a_center;
layout(location = 3) in float a_angle; 
layout(location = 4) in float a_amplitude;      // Wave amplitude 
layout(location = 5) in float a_frequency;      // Wave frequency
layout(location = 6) in float a_offset;      // Wave frequency

// Data for entire draw
uniform mat4 u_viewProjectionMatrix;
uniform float u_t; // Time for equations

// Output to fragment shader (textures not modified here)
out vec2 v_texCoord;

const float PI = 3.14159;

void main() {
    float c = cos(a_angle);
    float s = sin(a_angle);

    // Rotate the mesh but we still need to translate it
    vec2 rotatedPos;
    rotatedPos.x = a_localPos.x * c - a_localPos.y * s;
    rotatedPos.y = a_localPos.x * s + a_localPos.y * c;
    
    // Translate mesh to its position
    vec2 worldPos = a_center + rotatedPos;
    // Oscillating motion of the particle, we use 2 * PI to normalize a frequency of 1 to mean 1 cycle every 1 second
    worldPos.x = worldPos.x + (a_amplitude * sin(2 * PI * a_frequency * u_t + a_offset));
    
    gl_Position = u_viewProjectionMatrix * vec4(worldPos, 0.0, 1.0);
    v_texCoord = a_texCoord;
}