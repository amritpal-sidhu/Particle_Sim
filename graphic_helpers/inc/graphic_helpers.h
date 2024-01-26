#pragma once

#define GLFW_INCLUDE_NONE

#include <stdio.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include "vector.h"


#define PI                  3.14159265358979323846264338327950

#define VERTEX_SHADER_FILEPATH      "shaders/vs.vert.glsl"
#define FRAGMENT_SHADER_FILEPATH    "shaders/fs.frag.glsl"

#define VERT_POS_VAR                "position"
#define VERT_COL_VAR                "color"
#define VERT_POS_LOC                0
#define VERT_COL_LOC                1
#define UNIFORM_MVP_VAR             "MVP"
#define UNIFORM_MVP_LOC             2

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

struct draw_variables
{
    float ratio;
    int num_segments;
    double view_scalar;
    vector3d_t pos;
    vector3d_t angle;
};


/* Soley for visually identifying the particles graphically */
extern const color_t p_color;
extern const color_t e_color;


int shader_compile_and_link(GLuint *program);

void enable_vertex_attributes(void);

void vertex_array_object_init(GLuint *VAO, GLuint *VBO, const size_t VBO_count);

void vertex_buffer_init(GLuint *VBO, const struct vertex *vertices, const int v_size);
void vertex_buffer_draw(const GLuint program, const struct draw_variables draw_vars);

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const double r, const int num_segments, const color_t color);
void create_sphere_vertex_array(struct vertex *v, const vector3d_t center, const double r, const int num_y_segments, const int num_z_segments, const color_t color);
