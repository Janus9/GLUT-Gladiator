#version 330 core

layout(location = 0) in vec2 a_localPos;    // More of like the size
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec2 a_center;

uniform mat4 u_viewProjectionMatrix;

out vec2 v_texCoord;

void main() {
    vec2 modelPos = a_localPos + a_center;

    v_texCoord = a_texCoord;
    gl_Position = u_viewProjectionMatrix * vec4(modelPos, 0.0, 1.0);
}