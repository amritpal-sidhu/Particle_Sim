#pragma once

#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include "vector.h"


#define PI                  3.14159265358979323846264338327950

#define VERTEX_SHADER_FILEPATH      "shaders/vs.vert.glsl"
#define FRAGMENT_SHADER_FILEPATH    "shaders/fs.frag.glsl"

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
    struct shader_variables shader_vars;
    vector3d_t pos;
    vector3d_t angle;
};


/* Soley for visually identifying the particles graphically */
extern const color_t p_color;
extern const color_t e_color;


int shader_compile_and_link(GLuint *program);

void vertex_buffer_init(GLuint *VBO, const struct vertex *vertices, const int v_size);
void vertex_buffer_draw(const GLuint VBO, const struct draw_variables draw_vars);

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const double r, const int num_segments, const color_t color);
void create_sphere_vertex_array(struct vertex *v, const vector3d_t center, const double r, const int num_y_segments, const int num_z_segments, const color_t color);
