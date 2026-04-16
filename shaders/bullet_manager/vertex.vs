#version 120

/*
Attribute - Per vertex data comming from the VBO
Unform - Data for the whole draw
Varying - Output from vertex shader for fragment shader (usually texture coords)
*/

// Data from the VBO //
attribute vec2 a_localPos;
attribute vec2 a_texCoord;
attribute vec2 a_center;
attribute float a_angle; 

// Data for entire draw
uniform vec4 u_dimensions; // Left, right, top, bottom

// Output to fragment shader (textures not modified here)
varying vec2 v_texCoord;

void main() {
    float c = cos(a_angle);
    float s = sin(a_angle);

    // Rotate the mesh but we still need to translate it
    vec2 rotatedPos;
    rotatedPos.x = a_localPos.x * c - a_localPos.y * s;
    rotatedPos.y = a_localPos.y * s + a_localPos.y * c;
    
    // Translate mesh to its position
    vec2 worldPos = a_center + rotatedPos;
    
    // GPU coordinates use [-1 to 1] for drawing. So we need to normalize first
    vec2 clipPos;
    clipPos.x = ((worldPos.x - u_dimensions.x) / (u_dimensions.y - u_dimensions.x)) * 2.0 - 1.0;
    clipPos.y = ((worldPos.y - u_dimensions.w) / (u_dimensions.z - u_dimensions.w)) * 2.0 - 1.0;

    // gl_Position = (x,y,z,w) as homogenous coordinates.
    gl_Position = vec4(clipPos.x, clipPos.y, 0.0, 1.0); // No Z component so 0.0 and W should generally be 1.0
    v_texCoord = a_texCoord;
}