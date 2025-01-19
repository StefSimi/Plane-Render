#version 330 core

out vec4 out_Color;
uniform bool isSnow;

void main() {
    if (isSnow) {
        // Snow color: white and slightly transparent
        out_Color = vec4(1.0, 1.0, 1.0, 0.8);
    } else {
        // Rain color: light blue and slightly transparent
        out_Color = vec4(0.5, 0.5, 1.0, 0.8);
    }
}
