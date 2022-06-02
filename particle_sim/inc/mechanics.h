#pragma once

#include <stdlib.h>

#include "particle.h"
#include "vector.h"


double gravitational_force(const double m1, const double m2, const double r);
double electric_force(const double q1, const double q2, const double r);

/**
 *  TODO: look into these more
 */
void update_momentum(vector3d_t *momentum_integral, const vector3d_t F, const double sample_period);
void update_positions(particle_t **particles, const size_t particle_count, const double sample_period);
