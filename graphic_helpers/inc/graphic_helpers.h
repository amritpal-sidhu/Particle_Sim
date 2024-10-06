#pragma once

#include <stdio.h>

#include "linmath.h"

#include "vector.h"
#include "common.h"

#define PI                  3.14159265358979323846264338327950

#define POS_ATTR_LOC                0
#define COL_ATTR_LOC                1
#define MVP_UNION_LOC               2

#define DEBUG_OUTPUT_FILEPATH       "debug_output.txt"


typedef enum {
    P_BUF,
    E_BUF,
} buffer_index_e;

#define BO_COUNT   2

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

struct render_data_s
{
    float ratio;
    int num_segments;
    double view_scalar;
    GLuint VAO[BO_COUNT];
    GLuint VBO[BO_COUNT];
    GLuint program;
};


/* Soley for visually identifying the particles graphically */
extern const color_t p_color;
extern const color_t e_color;


void vertex_buffer_init(GLuint *VBO, void *data, const size_t size);
void vertex_array_object_init(struct render_data_s *rdata);
void bind_vertex_attributes(const struct render_data_s *rdata);

int shader_compile_and_link(struct render_data_s *rdata);
int shader_compile_and_link_spir_v(struct render_data_s *rdata);
void update_mvp_uniform(struct render_data_s *rdata, const vector3d_t pos, const vector3d_t angle);

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const double r, const int num_segments, const color_t color);
void create_sphere_vertex_array(struct vertex *v, const vector3d_t center, const double r, const int num_y_segments, const int num_z_segments, const color_t color);
