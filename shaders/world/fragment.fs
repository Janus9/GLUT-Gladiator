#version 330 core

// From the vertex shader
in vec2 v_atlasUV;  
flat in float v_selected;
in vec2 v_modelPos;

// sampler2D is a reference to a 2D image in GPU memory (SOIL stores images in GPU!)
uniform sampler2D u_texture;
uniform vec2 u_cameraPos;
uniform float u_time;

// This can be any name
out vec4 FragColor;

void main() {
    const vec3 highlight = vec3(1.0,1.0,0.4); // Warm yellow
    
    vec4 baseColor = texture(u_texture, v_atlasUV);
    
    const vec2 atlasGridSize = vec2(28.0, 20.0); 
    vec2 uv = fract(v_atlasUV * atlasGridSize);

    float dist = distance(v_modelPos, u_cameraPos); // Distance from pixel to camera
    float maxDist = 300.0;
    float brightness = 1.0 - clamp(dist / maxDist, 0.0, 1.0); // Force X to stay within min, max value
    brightness = brightness * brightness;

    brightness = mix(0.05,1.0,brightness);

    baseColor.rgb *= brightness;

    if (v_selected > 0.0) {
        // Selected
        // baseColor.rgb = mix(baseColor.rgb, highlight, 0.35);
        
        // check for border 
        const float outlinePct = 0.1;
        if ((uv.x > 0.0 && uv.x < outlinePct) ||          // Left Border
            (uv.x > 1.0 - outlinePct && uv.x < 1.0) ||    // Right Border
            (uv.y > 0.0 && uv.y < outlinePct) ||          // Top Border
            (uv.y > 1.0 - outlinePct && uv.y < 1.0))      // Bottom Border
        {
            baseColor.rgb = vec3(0.0,0.0,0.0);
        }
    } 

    FragColor = baseColor;
}