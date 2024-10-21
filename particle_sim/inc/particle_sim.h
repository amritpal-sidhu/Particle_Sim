#pragma once

#include "common.h"
#include "graphic_helpers.h"
#include "particle.h"
#include "mechanics.h"
#include "vector.h"


#define DRAW_SPHERE
#define CIRCLE_Y_SEGMENTS       64
#define CIRCLE_Z_SEGMENTS       64
#ifdef DRAW_SPHERE
#define NUM_SEGMENTS            (CIRCLE_Y_SEGMENTS * CIRCLE_Z_SEGMENTS + 2)
#else
#define NUM_SEGMENTS            CIRCLE_Y_SEGMENTS
#endif

#define P_COUNT             1   // Temporary solution to "simulate" a nucleus
#define E_COUNT             2
#define NUM_PARTICLES       (P_COUNT + E_COUNT)


/* Main parameters that will effect the behavior */
static const float sample_period = 8E-3f;

static const vector3d_t initial_pos[NUM_PARTICLES] = {
    /* Positively charged */
    {.i = 0.0f, .j = 0.0f, .k = 0.0f},
    /* Negatively charged */
    {.i = 0.3f, .j = 0.5f, .k = 0.0f},
    {.i = 0.5f, .j = 0.3f, .k = 0.0f},
};

static const vector3d_t initial_momentum[NUM_PARTICLES] = {
    /* Positively charged */
    {.i = 0.0f, .j = 0.0f, .k = 0.0f},
    /* Negatively charged */
    {.i = 0.0f, .j = 0.0f, .k = 0.0f},
    {.i = 0.0f, .j = 0.0f, .k = 0.0f},
};

static const vector3d_t initial_orientation[NUM_PARTICLES] = {
    /* Positively charged */
    {.i = 0.0f, .j = 0.0f, .k = 0.0f},
    /* Negatively charged */
    {.i = 0.0f, .j = 0.0f, .k = 0.0f},
    {.i = 0.0f, .j = 0.0f, .k = 0.0f},
};

static const vector3d_t initial_angular_momentum[NUM_PARTICLES] = {
    /* Positively charged */
    {.i = 0.0f, .j = 0.0f, .k = 0.0f},
    /* Negatively charged */
    {.i = 0.0f, .j = 0.0f, .k = 0.0f},
    {.i = 0.0f, .j = 0.0f, .k = 0.0f},
};


extern log_t *log_handle;
extern particle_t particles[NUM_PARTICLES];
extern struct render_data_s rdata;


void opengl_libraries_init(GLFWwindow **window);

void create_particle_vertices(struct vertex *p_vertices, struct vertex *e_vertices);
void create_particle_objects(particle_t *particles);

void clean_program(GLFWwindow *window);

/* GLFW callbacks */
#ifdef DEBUG
void debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
void pre_call_gl_callback(const char *name, GLADapiproc apiproc, int len_args, ...);
void post_call_gl_callback(void *ret, const char *name, GLADapiproc apiproc, int len_args, ...);
#endif
void error_callback(int error, const char *description);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
