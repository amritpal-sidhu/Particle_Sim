#include "particle.h"

#include <stdlib.h>


void particle__init(particle_t *p, const unsigned long long int id,
                          const vector3d_t initial_pos, const vector3d_t initial_momentum,
                          const vector3d_t initial_orientation, const vector3d_t initial_angular_momentum,
                          const float mass, const float charge, const float radius)
{
    p[id].id = id;
    p[id].pos = initial_pos;
    p[id].momenta = initial_momentum;
    p[id].orientation = initial_orientation;
    p[id].angular_momenta = initial_angular_momentum;
    p[id].mass = mass;
    p[id].charge = charge;
    p[id].radius = radius;
}
