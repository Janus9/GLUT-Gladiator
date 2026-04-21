#version 120

uniform sampler2D u_texture; // Grab image from CPU
uniform double u_time; // Grab time (in seconds) from CPU

varying vec2 v_texcoord;
varying vec4 v_color

vec3 hsv2rgb(float h) {
    vec3 aq = vec3(h,h + 1.0/3.0, h+2.0/3.0);
    return 0.5 + 0.5 * cos(6.28318 * (aq));
}

void main() {

    // 1. Calculate a 'hue' value (0.0 to 1.0)

    // We use the particle's screen position + time for the rainbow shift

    float hue = (gl_FragCoord.x * 0.002) + (u_time * 0.5);

    vec3 rainbow = hsv2rgb(hue);

    // 2. Sample texture & Calculate Radial Glow

    vec4 texColor = texture2D(u_texture, v_texcoord);

    float dist = length(v_texcoord - 0.5);

    float glow = 0.07 / dist;

    // 3. Combine: Texture * Rainbow * Glow

    // Multiply by v_color.rgb if you want to tint from the CPU side too

    vec3 finalRGB = texColor.rgb * rainbow * glow * v_color.rgb;

    // 4. Alpha masking (keeps the shape of your texture)

    float alpha = texColor.a * v_color.a * smoothstep(0.5, 0.2, dist);

    gl_FragColor = vec4(finalRGB, alpha);
}

