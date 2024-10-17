#version 460 core


layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) uniform mat4 MVP;

out VS_OUT
{
    vec3 color;
} vs_out;


void main()
{
    gl_Position = MVP * vec4(in_pos, 1);
    vs_out.color = in_color;
}
