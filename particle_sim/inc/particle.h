#pragma once

#include "vector.h"

#define PROTON_CHARGE       1.602E-19  // Coulombs
#define ELECTRON_CHARGE     -1.602E-19

#define PROTON_MASS         1.6727E-24 // Grams
#define NEUTRON_MASS        1.675E-24
#define ELECTRON_MASS       9.11E-28

typedef struct particle
{
    int id;
    vector_3d_t pos;
    vector_3d_t vel;
    double mass;
    double charge;

} particle_t;
