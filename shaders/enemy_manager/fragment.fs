#version 330 core

// From the vertex shader
in vec2 v_texCoord;

// sampler2D is a reference to a 2D image in GPU memory (SOIL stores images in GPU!)
uniform float u_time;
uniform sampler2D u_texture;

// This can be any name
out vec4 FragColor;

void main() {
    FragColor = texture(u_texture,v_texCoord);
}