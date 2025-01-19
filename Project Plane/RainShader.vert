#version 330 core
layout(location = 0) in vec3 in_Position;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(in_Position, 1.0);
    gl_PointSize = 4.0; // Set size for point rendering
}
