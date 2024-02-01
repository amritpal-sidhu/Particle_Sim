#version 460 core

layout(location = 0) in dvec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) uniform mat4x4 in_MVP;
out vec3 vertex_color;

void main()
{
    gl_Position = in_MVP * vec4(in_pos, 1.0);
    vertex_color = in_color;
}
