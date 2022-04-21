#pragma once

#define UNIVERSAL_GRAVITY_CONST     6.673E-11 // (N*m^2)/(kg^2)
#define COULOMB_CONST               8.9875E9  // (N*m^2)/(C^2)

double gravitational_force(const double m1, const double m2, const double r);
double electric_force(const double q1, const double q2, const double r);

// TODO: look into this and name change
double velocity_induced_by_force(const double F, const double m, const double sample_period);
void clear_impulse_integral(void);
