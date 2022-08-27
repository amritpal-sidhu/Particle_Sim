#include "particle.h"

#include <stdlib.h>


particle_t *particle__new(const unsigned long long int id,
                          const vector3d_t initial_pos, const vector3d_t initial_momentum,
                          const vector3d_t initial_spin, const vector3d_t initial_angular_momentum,
                          const double mass, const double charge, const double radius)
{
    particle_t *p = malloc(sizeof(particle_t));

    if (p) {
        p->id = id;
        p->pos = initial_pos;
        p->momenta = initial_momentum;
        p->spin = initial_spin;
        p->angular_momenta = initial_angular_momentum;
        p->mass = mass;
        p->charge = charge;
        p->radius = radius;
    }

    return p;
}

void particle__delete(particle_t *p)
{
    free(p);
}
