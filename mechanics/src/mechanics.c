#include "mechanics.h"

#include <math.h>

#include "log.h"


// #define __USE_GRAVITY

#define LOCAL_EPSILON               1E-128

#define UNIVERSAL_GRAVITY_CONST     6.6743E-17 // (N*m^2)/(g^2)
#define COULOMB_CONST               8.9875E9  // (N*m^2)/(C^2)


extern log_t *log_handle;


/* Public function definitions */
double gravitational_force(const double m1, const double m2, double r)
{
    if (r < LOCAL_EPSILON) r = LOCAL_EPSILON;

    return (UNIVERSAL_GRAVITY_CONST * m1 * m2) / (r * r);
}

double electric_force(const double q1, const double q2, double r)
{
    if (r < LOCAL_EPSILON) r = LOCAL_EPSILON;

    return (COULOMB_CONST * q1 * q2) / (r * r);
}

vector2d_t componentize_force_2d(const double F, const vector2d_t direction_vector)
{
    const double azimuth_cos = acos(direction_vector.i / vector2d__mag(direction_vector));
    const double azimuth_sin = asin(direction_vector.j / vector2d__mag(direction_vector));

    vector2d_t force = {
        .i = F * cos(azimuth_cos),
        .j = F * sin(azimuth_sin)
    };

    return force;
}

vector3d_t componentize_force_3d(const double F, const vector3d_t direction_vector)
{
    const double magnitude = vector3d__mag(direction_vector);
    const double polar_cos = acos(direction_vector.k / magnitude);
    const double polar_sin = asin(vector2d__mag((vector2d_t){.i = direction_vector.i, .j = direction_vector.j}) / magnitude);
    const double azimuth_cos = acos(direction_vector.i / (magnitude * sin(polar_sin)));
    const double azimuth_sin = asin(direction_vector.j / (magnitude * sin(polar_sin)));

    vector3d_t force = {
        .i = F * cos(azimuth_cos) * sin(polar_sin),
        .j = F * sin(azimuth_sin) * sin(polar_sin),
        .k = F * cos(polar_cos)
    };

    return force;
}

void update_momentum(vector3d_t *momentum_integral, const vector3d_t F, const double sample_period)
{
    *momentum_integral = vector3d__add(*momentum_integral, vector3d__scale(F, sample_period));
}

void update_positions(particle_t **particles, const size_t particle_count, const double sample_period)
{
    for (size_t this = 0; this < particle_count; ++this) {

        vector3d_t F_resultant = {0};

        /* Try to find a time improvement to compute all forces acting on current particle */
        for (size_t that = 0; that < particle_count; ++that) {

            if (particles[this]->id == particles[that]->id) continue;

            const double r = vector3d__distance(particles[this]->pos, particles[that]->pos);

            F_resultant = vector3d__add(
                F_resultant,
                componentize_force_3d(
                    electric_force(particles[this]->charge, particles[that]->charge, r),
                    vector3d__sub(particles[this]->pos, particles[that]->pos)
                )
            );
            #ifdef __USE_GRAVITY
            F_resultant = vector3d__add(
                F_resultant,
                componentize_force_3d(
                    gravitational_force(particles[this]->mass, particles[that]->mass, r),
                    vector3d__sub(particles[that]->pos, particles[this]->pos)
                )
            );
            #endif
        }

        update_momentum(&particles[this]->momenta, F_resultant, sample_period);
        const vector3d_t change_in_velocity = vector3d__scale(particles[this]->momenta, 1 / particles[this]->mass);

        particles[this]->pos = vector3d__add(particles[this]->pos, vector3d__scale(change_in_velocity, sample_period));

        log__write(log_handle, LOG_DATA, "%i,%E,%E,%E,%E,%E,%f,%f,%f",
        particles[this]->id, particles[this]->mass, particles[this]->charge, 
        particles[this]->momenta.i, particles[this]->momenta.j, particles[this]->momenta.k,
        particles[this]->pos.i, particles[this]->pos.j, particles[this]->pos.k);
    }

    log__write(log_handle, LOG_NONE, "");
}
