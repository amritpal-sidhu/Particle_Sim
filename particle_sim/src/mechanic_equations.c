#include "mechanic_equations.h"


double gravitational_force(const double m1, const double m2, const double r)
{
    return (UNIVERSAL_GRAVITY_CONST * m1 * m2) / (r * r);
}

double electric_force(const double q1, const double q2, const double r)
{
    return (COULOMB_CONST * q1 * q2) / (r * r);
}

void update_momentum(vector3d_t *momentum_integral, const vector3d_t F, const double sample_period)
{
    *momentum_integral = vector3d__add(*momentum_integral, vector3d__scale(F, sample_period));
}
