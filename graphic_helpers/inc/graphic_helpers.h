#pragma once

#include <stdio.h>

#include "linmath.h"

#include "vector.h"
#include "common.h"

#define PI                  3.14159265358979323846264338327950f

/* shader storage buffer binding point */
#define SSBO_BINDING_POINT          0

/* uniform buffer object offsets and sizes */
#define UBO_VIEW_SCALAR_OFFSET      0
#define UBO_VIEW_RATIO_OFFSET       16
#define UBO_SIZE                    32
/* shader storage buffer offsets and sizes */
#define SSBO_INDEX_OFFSET           0
#define SSBO_SAMPLE_PERIOD_OFFSET   sizeof(unsigned int)
#define SSBO_PARTICLES_OFFSET       sizeof(float)+SSBO_SAMPLE_PERIOD_OFFSET
#define SSBO_SIZE                   sizeof(particle_t)*(P_COUNT+E_COUNT)+SSBO_PARTICLES_OFFSET

/* vertex shader locations */
#define UBO_BINDING_POINT           1
#define VS_IN_POS_LOC               0
#define VS_IN_COLOR_LOC             1

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

struct render_data_s
{
    float ratio;
    int num_segments;
    float view_scalar;
    GLuint VAO[VBO_COUNT];
    GLuint VBO[VBO_COUNT];
    GLuint UBO;
    GLuint SSBO;
    GLuint program;
    GLuint compute_program;
};


/* Soley for visually identifying the particles graphically */
extern const color_t p_color;
extern const color_t e_color;


void vertex_buffer_init(struct render_data_s *rdata, const buffer_index_e buf, void *data, const size_t size);
void shader_storage_buffer_init(struct render_data_s *rdata, void *particle_data);
void uniform_buffer_init(struct render_data_s *rdata);
void vertex_array_object_init(struct render_data_s *rdata);
void bind_vertex_array(const struct render_data_s *rdata);

int shader_compile_and_link(struct render_data_s *rdata);
int shader_compile_and_link_spir_v(struct render_data_s *rdata);
void render_particles(const struct render_data_s *rdata, const unsigned int particle_index);
void run_time_evolution_shader(const struct render_data_s *rdata, const unsigned int particle_index);

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const float r, const int num_segments, const color_t color);
void create_sphere_vertex_array(struct vertex *v, const vector3d_t center, const float r, const int num_y_segments, const int num_z_segments, const color_t color);
