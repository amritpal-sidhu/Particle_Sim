#version 460

in dvec3 position;
in vec3 color;
out vec3 vertex_color;
uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(position, 1.0);
    vertex_color = color;
}
