#include "mechanic_equations.h"


double gravitational_force(const double m1, const double m2, const double r)
{
    return (UNIVERSAL_GRAVITY_CONST * m1 * m2) / (r * r);
}

double electric_force(const double q1, const double q2, const double r)
{
    return (COULOMB_CONST * q1 * q2) / (r * r);
}

vector3d_t velocity_induced_by_force(vector3d_t *impulse_integral, const vector3d_t F, const double m, const double sample_period)
{
    *impulse_integral = vector_3d__add(*impulse_integral, vector_3d__scale(F, sample_period));

    return vector_3d__scale(*impulse_integral, 1/m);
}
