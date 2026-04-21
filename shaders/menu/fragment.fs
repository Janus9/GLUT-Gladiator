#version 330 core

in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform bool u_isHovering;      // Boolean if the mouse is hovering the current object

out vec4 FragColor;

void main() {
    vec4 texColor = texture2D(u_texture,v_texCoord); // Gets the current texture color
    if (u_isHovering) {
        // Apply 20% tint darker
        texColor.rgb = max(texColor.rgb - 0.2, 0.0); // Min to not exceed 0.0
    }
    FragColor = texColor;
}