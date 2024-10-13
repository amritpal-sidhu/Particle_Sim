#pragma once

#include <stdio.h>

#include "linmath.h"

#include "vector.h"
#include "common.h"

#define PI                  3.14159265358979323846264338327950f

/* compute shader locations */
#define CS_ID_UNIFORM_LOC           0
#define SAMPLE_PERIOD_UNIFORM_LOC   1

/* vertex shader locations */
#define IN_POS_ATTR_LOC             0
#define IN_COL_ATTR_LOC             1
#define VS_ID_UNIFORM_LOC           2
#define VIEW_SCALAR_UNIFORM_LOC     3
#define VIEW_RATIO_UNIFORM_LOC      4

/* shader storage buffer binding point */
#define SSBO_BINDING_POINT          0

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
    GLuint SSBO;
    GLuint SSBO_block_index[2];
    GLuint program;
    GLuint compute_program;
};


/* Soley for visually identifying the particles graphically */
extern const color_t p_color;
extern const color_t e_color;


void vertex_buffer_init(GLuint *VBO, void *data, const size_t size);
void vertex_array_object_init(struct render_data_s *rdata);
void bind_vertex_attributes(const struct render_data_s *rdata);
void shader_storage_buffer_init(struct render_data_s *rdata, void *data, const size_t size);

int shader_compile_and_link(struct render_data_s *rdata);
int shader_compile_and_link_spir_v(struct render_data_s *rdata);
void render_particles(const struct render_data_s *rdata, const size_t particle_index);
void run_time_evolution_shader(const struct render_data_s *rdata, const size_t particle_index, const float sample_period);

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const double r, const int num_segments, const color_t color);
void create_sphere_vertex_array(struct vertex *v, const vector3d_t center, const double r, const int num_y_segments, const int num_z_segments, const color_t color);
