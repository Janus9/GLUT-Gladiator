#version 330 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_texCoord;

uniform mat4 u_projection;
uniform mat4 u_model;

out vec2 v_texCoord;

void main() {
    v_texCoord = a_texCoord;
    
    // View is identity matrix
    gl_Position = u_projection * u_model * vec4(a_pos, 0.0, 1.0);
}