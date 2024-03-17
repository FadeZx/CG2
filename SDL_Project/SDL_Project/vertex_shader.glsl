#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 c_pos;
out vec4 vertex_color;

void main() {
    gl_Position = vec4(a_pos, 1.0);
    vertex_color = vec4(c_pos, 1.0);
}
