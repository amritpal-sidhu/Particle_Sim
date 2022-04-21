#include "mechanic_equations.h"


static double impulse_integral;


double gravitational_force(const double m1, const double m2, const double r)
{
    return (UNIVERSAL_GRAVITY_CONST * m1 * m2) / (r * r);
}

double electric_force(const double q1, const double q2, const double r)
{
    return (COULOMB_CONST * q1 * q2) / (r * r);
}

double velocity_induced_by_force(const double F, const double m, const double sample_period)
{
    impulse_integral += F * sample_period;

    return impulse_integral / m;
}

void clear_impulse_integral(void)
{
    impulse_integral = 0;
}
