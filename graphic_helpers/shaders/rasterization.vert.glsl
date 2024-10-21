#version 460 core


layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) uniform uint index;
layout(std430, binding = 1) buffer MVP_data_block
{
    mat4 MVP[];
};

out VS_OUT
{
    vec3 color;
} vs_out;


void main()
{
    gl_Position = MVP[index] * vec4(vertex_pos, 1);
    vs_out.color = vertex_color;
}
