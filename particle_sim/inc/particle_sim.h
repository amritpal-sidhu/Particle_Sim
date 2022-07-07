#pragma once

#include "vector.h"


#define __DRAW_SPHERE
#define CIRCLE_Y_SEGMENTS       64
#define CIRCLE_Z_SEGMENTS       64
#ifdef __DRAW_SPHERE
#define NUM_SEGMENTS            (CIRCLE_Y_SEGMENTS * CIRCLE_Z_SEGMENTS) + 2
#else
#define NUM_SEGMENTS            CIRCLE_Y_SEGMENTS
#endif

#define P_COUNT             1   // Temporary solution to "simulate" a nucleus
#define E_COUNT             2


/* Main parameters that will effect the behavior */
static const double sample_period = 8E-3;

static const vector3d_t initial_pos[P_COUNT+E_COUNT] = {
    /* Positively charged */
    {.i = 0, .j = 0, .k = 0},
    /* Negatively charged */
    {.i = 0.3, .j = 0.5, .k = 0},
    {.i = 0.5, .j = 0.3, .k = 0},
};

static const vector3d_t initial_momentum[P_COUNT+E_COUNT] = {
    /* Positively charged */
    {.i = 0, .j = 0, .k = 0},
    /* Negatively charged */
    {.i = 0, .j = 0, .k = 0},
    {.i = 0, .j = 0, .k = 0},
};

static const vector3d_t initial_spin[P_COUNT+E_COUNT] = {
    /* Positively charged */
    {.i = 0, .j = 0, .k = 0},
    /* Negatively charged */
    {.i = 0, .j = 0, .k = 0},
    {.i = 0, .j = 0, .k = 0},
};

static const vector3d_t initial_angular_momentum[P_COUNT+E_COUNT] = {
    /* Positively charged */
    {.i = 0, .j = 0, .k = 0},
    /* Negatively charged */
    {.i = 0, .j = 0, .k = 0},
    {.i = 0, .j = 0, .k = 0},
};
