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

// Data for entire draw
uniform mat4 u_viewProjectionMatrix;

// Output to fragment shader (textures not modified here)
out vec2 v_texCoord;
out vec2 v_modelPos;

void main() {
    float c = cos(a_angle);
    float s = sin(a_angle);

    // Rotate the mesh but we still need to translate it
    vec2 rotatedPos;
    rotatedPos.x = a_localPos.x * c - a_localPos.y * s;
    rotatedPos.y = a_localPos.x * s + a_localPos.y * c;
    
    // Translate mesh to its position
    vec2 worldPos = a_center + rotatedPos;
    
    v_texCoord = a_texCoord;
    v_modelPos = worldPos;

    gl_Position = u_viewProjectionMatrix * vec4(worldPos, 0.0, 1.0);
}