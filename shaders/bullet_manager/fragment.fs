#version 120

// sampler2D is a reference to a 2D image in GPU memory (SOIL stores images in GPU!)
uniform sampler2D u_texture;

varying vec2 v_texCoord;

void main() {
    gl_FragColor = texture2D(u_texture,v_texCoord);
}