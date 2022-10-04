#pragma once

#include "vector.h"


#define PROTON_CHARGE       1.602E-19  // Coulombs
#define ELECTRON_CHARGE     -1.602E-19

#define PROTON_MASS         1.6727E-24 // Grams
#define NEUTRON_MASS        1.675E-24
#define ELECTRON_MASS       9.11E-28

/* Based on Wikipedia covalent radi */
#define HELIUM_NUCLEUS_RADIUS   28E-12     // Meters
#define ELECTRON_RADIUS         10E-15      // Meters
#define FAKE_NUCLEUS_RADIUS     0.1


typedef struct
{
    unsigned long long int id;
    vector3d_t pos;
    vector3d_t momenta;
    vector3d_t orientation;
    vector3d_t angular_momenta;
    double mass;
    double charge;
    double radius;

} particle_t;


particle_t *particle__new(const unsigned long long int id,
                          const vector3d_t initial_pos, const vector3d_t initial_momentum,
                          const vector3d_t initial_orientation, const vector3d_t initial_angular_momentum,
                          const double mass, const double charge, const double radius);
void particle__delete(particle_t *p);
