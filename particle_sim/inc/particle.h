#pragma once

#include "vector.h"

#define PROTON_CHARGE       1.602E-19  // Coulombs
#define ELECTRON_CHARGE     -1.602E-19

#define PROTON_MASS         1.6727E-24 // Grams
#define NEUTRON_MASS        1.675E-24
#define ELECTRON_MASS       9.11E-28

typedef struct particle
{
    unsigned long long int id;
    vector3d_t pos;
    vector3d_t momentum_integral;
    double mass;
    double charge;

} particle_t;


particle_t *particle__new(const unsigned long long int id, const vector3d_t initial_pos, const vector3d_t initial_momentum, const double mass, const double charge);
void particle__delete(particle_t *p);
