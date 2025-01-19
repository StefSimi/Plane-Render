#version 330 core

out vec4 out_Color;

uniform vec4 nightColor;
uniform vec4 dawnColor;
uniform vec4 noonColor;
uniform float timeFactor; // Normalized time of day (0.0 - 1.0)

void main() {
    // Blend colors based on fragment height
    float height = gl_FragCoord.y / 600.0; // Assuming a window height of 600
    vec4 gradientColor;

    if (timeFactor < 0.3) {
        gradientColor = mix(nightColor, dawnColor, timeFactor / 0.3);
    } else {
        gradientColor = mix(dawnColor, noonColor, (timeFactor - 0.3) / 0.7);
    }

    // Blend the gradient with the height to create a vertical transition
    out_Color = mix(vec4(gradientColor.rgb, 1.0), vec4(gradientColor.rgb * 0.5, 1.0), height);
}
