#pragma once

#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include "vector.h"


#define PI                  3.14159265358979323846264338327950

#define VERTEX_SHADER_FILEPATH      "shaders/vs.vert"
#define FRAGMENT_SHADER_FILEPATH    "shaders/fs.frag"

#define DEBUG_OUTPUT_FILEPATH       "debug_output.txt"


typedef struct
{
    float r;
    float g;
    float b;

} color_t;

struct vertex
{
    vector3d_t pos;
    color_t color;
};

struct shader_variables
{
    GLint vpos_location;
    GLint vcol_location;
    GLint mvp_location;
};

struct draw_variables
{
    float ratio;
    int num_segments;
    double view_scalar;
};


extern FILE *debug_fp;


int shader_compile_and_link(GLuint *program);

void vertex_buffer_init(GLuint *VBO, const struct vertex *vertices, const int v_size);
void vertex_buffer_draw(const GLuint VBO, const struct shader_variables shader_vars, const struct draw_variables draw_vars, const vector3d_t pos);

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const float r, const int num_segments, const color_t color);
void busy_wait_ms(const float delay_in_ms);
