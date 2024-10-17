#pragma once

#include <stdio.h>

#include "linmath.h"

#include "particle.h"
#include "common.h"

#include "vector.h"


#define PI                  3.14159265358979323846264338327950f

#define P_COUNT             1   // Temporary solution to "simulate" a nucleus
#define E_COUNT             2


/* compute shader locations */
#define SSBO_BINDING_POINT          0
#define UBO_BINDING_POINT           1
#define CS_MVP_UNIFORM_LOC          0

/* vertex shader locations */
#define VS_IN_POS_LOC               0
#define VS_IN_COLOR_LOC             1
#define VS_MVP_UNIFORM_LOC          2

#define DEBUG_OUTPUT_FILEPATH       "debug_output.txt"


typedef enum {
    P_BUF,
    E_BUF,
} buffer_index_e;

#define VBO_COUNT   2

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

typedef enum
{
    TIME_EVOLVE,
    RASTER,
} program_e;

#define PROGRAM_COUNT   2

struct render_data_s
{
    int width;
    int height;
    float ratio;
    int num_segments;
    float view_scalar;
    GLuint VAO[VBO_COUNT];
    GLuint VBO[VBO_COUNT];
    GLuint SSBO;
    GLuint UBO;
    GLuint program[PROGRAM_COUNT];
    GLfloat MVP[P_COUNT+E_COUNT][4][4];
};


/* Soley for visually identifying the particles graphically */
extern const color_t p_color;
extern const color_t e_color;

/* shader storage buffer offsets and size */
static struct ssbo_info_s
{
    unsigned int particles_offset;
    unsigned int size;
} ssbo_info;

/* uniform buffer offsets and size */
static struct ubo_info_s
{
    float sample_period_offset;
    float view_scalar_offset;
    float ratio_offset;
    unsigned int size;
} ubo_info;

void buffer_objects_init(struct render_data_s *rdata, void *p_verts, void *e_verts, void *particle_data);
void vertex_array_object_init(struct render_data_s *rdata);

int shader_compile_and_link(struct render_data_s *rdata);
int shader_compile_and_link_spir_v(struct render_data_s *rdata);

void update_mvp_uniform(struct render_data_s *rdata, const vector3d_t pos, const vector3d_t angle);
void render_particles(struct render_data_s *rdata, const unsigned int index, particle_t *particles);
void run_time_evolution_shader(struct render_data_s *rdata, void *particle_data);

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const float r, const int num_segments, const color_t color);
void create_sphere_vertex_array(struct vertex *v, const vector3d_t center, const float r, const int num_y_segments, const int num_z_segments, const color_t color);
