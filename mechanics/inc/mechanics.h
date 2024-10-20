#pragma once

#include <stdlib.h>

#include "particle.h"
#include "vector.h"


double gravitational_force(const double m1, const double m2, double r);
double electric_force(const double q1, const double q2, double r);

/**
 * These functions use the position vectors to convert a scalar
 * force value into rectangular components.  The force is assumed
 * to be originating form a radial uniformly distributed force
 * field.
 * 
 * @param F Scalar value of force
 * @param direction_vector difference vector pointing towards reference particle, indexed as "this"
 * @return Componentized form of the force
 */
vector2d_t componentize_force_2d(const double F, const vector2d_t direction_vector);
vector3d_t componentize_force_3d(const double F, const vector3d_t direction_vector);

/**
 *  TODO: look into these more
 */
void time_evolution(particle_t **particles, const size_t particle_count, const double sample_period);
int detect_collision(const particle_t *this, const particle_t *that);
