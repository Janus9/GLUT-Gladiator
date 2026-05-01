#version 330 core

layout(location = 0) in vec2 a_dimensions;    
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec2 a_localPos;
layout(location = 3) in float a_selected;    // Tile Selection >= 0 means false

uniform mat4 u_viewProjectionMatrix;
uniform vec2 u_cameraPos;
uniform float u_time;

out vec2 v_atlasUV;
flat out float v_selected;   // Fragment shader interpolates values unless flat is applied

void main() {
    vec2 modelPos = a_dimensions + a_localPos;

    v_atlasUV = a_texCoord;
    v_selected = a_selected;

    gl_Position = u_viewProjectionMatrix * vec4(modelPos, 0.0, 1.0);
}