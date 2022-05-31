#include "particle.h"

#include <stdlib.h>

particle_t *particle__new(const unsigned long long int id, const vector3d_t initial_pos, const vector3d_t initial_vel, const double mass, const double charge)
{
    particle_t *p = malloc(sizeof(particle_t));

    if (p) {
        p->id = id;
        p->pos = initial_pos;
        p->vel = initial_vel;
        p->mass = mass;
        p->charge = charge;
    }

    return p;
}

void particle__delete(particle_t *p)
{
    free(p);
}
