#version 120

varying vec2 v_texcoord;
varying vec4 v_color

void main() {
    v_texcoord = gl_MultiTexCoord0.xy; // Grab texture coordinates from CPU

    v_color = gl_Color; // Grab color values

    // Grab model view projectin matrix
    gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
}