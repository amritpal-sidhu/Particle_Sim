#pragma once

#include <stdio.h>

#include "linmath.h"

#include "particle.h"
#include "common.h"

#include "vector.h"


#define PI                  3.14159265358979323846264338327950f


/* vertex shader locations */
#define VERTEX_POS_LOC                  glGetAttribLocation(rdata->program[RASTER], "vertex_pos")
#define VERTEX_COLOR_LOC                glGetAttribLocation(rdata->program[RASTER], "vertex_color")
#define VERTEX_INDEX_LOC                glGetUniformLocation(rdata->program[RASTER], "index")

#define DEBUG_OUTPUT_FILEPATH       "debug_output.txt"


typedef enum {
    PARTICLE_SSBO,
    MVP_SSBO,
} ssbo_type_e;

#define SSBO_COUNT  2

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
    GLuint SSBO[SSBO_COUNT];
    GLuint UBO;
    GLuint program[PROGRAM_COUNT];
};


/* Soley for visually identifying the particles graphically */
extern const color_t p_color;
extern const color_t e_color;

/* shader storage buffer offsets and size */
static struct ssbo_info_s
{
    GLuint binding;
    GLuint offset;
    GLuint size;
} ssbo_info[SSBO_COUNT];

/* uniform buffer offsets and size */
static struct ubo_info_s
{
    GLuint binding;
    GLuint sample_period_offset;
    GLuint view_scalar_offset;
    GLuint ratio_offset;
    GLuint size;
} ubo_info;

void buffer_objects_init(struct render_data_s *rdata, void *p_verts, void *e_verts, void *particles);
void vertex_array_object_init(struct render_data_s *rdata);
void set_particle_ssbo_data(const struct render_data_s *rdata, const GLenum access, void *particles);

int shader_compile_and_link(struct render_data_s *rdata);
int shader_compile_and_link_spir_v(struct render_data_s *rdata);

void render_particles(struct render_data_s *rdata, const unsigned int index, particle_t *particles);
void run_time_evolution_shader(struct render_data_s *rdata, particle_t *particles);

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const float r, const int num_segments, const color_t color);
void create_sphere_vertex_array(struct vertex *v, const vector3d_t center, const float r, const int num_y_segments, const int num_z_segments, const color_t color);
