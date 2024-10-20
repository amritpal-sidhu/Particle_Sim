#include "mechanics.h"

#include <math.h>

#include "log.h"


// #define __USE_GRAVITY

#define LOCAL_EPSILON               1E-128

#define UNIVERSAL_GRAVITY_CONST     6.6743E-17 // (N*m^2)/(g^2)
#define COULOMB_CONST               8.9875E9  // (N*m^2)/(C^2)


extern log_t *log_handle;


/* Private function declarations */
static void update_momenta(particle_t *particle, const vector3d_t F, const double sample_period);
static void update_position(particle_t *particle, const double sample_period);
static void update_angular_momenta(particle_t *particle, const vector3d_t r, const vector3d_t momentum);
static void update_orientation(particle_t *particle, const double sample_period);
static vector3d_t resultant_force_from_fields(particle_t **particles, const size_t particle_count, const size_t this);
static void elastic_collision_linear_momenta_update(particle_t *this, particle_t *that);
static void update_angular_momenta_after_collision(particle_t *this, particle_t *that);

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



void time_evolution(particle_t **particles, const size_t particle_count, const double sample_period)
{
    for (size_t this = 0; this < particle_count; ++this) {

        update_momenta(particles[this], resultant_force_from_fields(particles, particle_count, this), sample_period);
        update_position(particles[this], sample_period);
        update_orientation(particles[this], sample_period);

        /* Simple check for collision with another particle and perform momentum update */
        for (size_t that = 0; that < particle_count; ++that) {
            
            if (particles[this]->id == particles[that]->id) continue;

            if (detect_collision(particles[this], particles[that])) {

                /* Unconserved angular momentum portion */
                update_angular_momenta_after_collision(particles[this], particles[that]);
                update_orientation(particles[this], sample_period);

                elastic_collision_linear_momenta_update(particles[this], particles[that]);
                update_position(particles[this], sample_period);
            }
        }

        log__write(log_handle, LOG_DATA, "%i,%E,%E,%E,%E,%E,%f,%f,%f,%E,%E,%E,%f,%f,%f",
        particles[this]->id, particles[this]->mass, particles[this]->charge, 
        particles[this]->momenta.i, particles[this]->momenta.j, particles[this]->momenta.k,
        particles[this]->pos.i, particles[this]->pos.j, particles[this]->pos.k,
        particles[this]->angular_momenta.i,particles[this]->angular_momenta.j,particles[this]->angular_momenta.k,
        particles[this]->orientation.i,particles[this]->orientation.j,particles[this]->orientation.k);
    }

    log__write(log_handle, LOG_NONE, "");
}

int detect_collision(const particle_t *this, const particle_t *that)
{
    return vector3d__distance(this->pos, that->pos) < (this->radius + that->radius);
}

/* Private function definitions */
static void update_momenta(particle_t *particle, const vector3d_t F, const double sample_period)
{
    particle->momenta = vector3d__add(particle->momenta, vector3d__scale(F, sample_period));
}

static void update_position(particle_t *particle, const double sample_period)
{
    const vector3d_t change_in_velocity = vector3d__scale(particle->momenta, 1 / particle->mass);
    particle->pos = vector3d__add(particle->pos, vector3d__scale(change_in_velocity, sample_period));
}

static void update_angular_momenta(particle_t *particle, const vector3d_t r, const vector3d_t momentum)
{
    /**
     * Violating conservation of momentum and angular momentum here for the sake of simplicity.
     * TODO: Don't violate the laws of nature!
     */
    particle->angular_momenta = vector3d__add(particle->angular_momenta, vector3d__cross_product(r, momentum));
}

static void update_orientation(particle_t *particle, const double sample_period)
{
    /**
     * Moment of inertia of a sphere about its axis is 4/5 M R^2
     * with respect to its surface is 7/5 M R^2
     */
    const double moment_of_inertia_of_a_sphere = 1.4 * particle->mass * particle->radius * particle->radius;
    const vector3d_t change_in_orientation = vector3d__scale(particle->angular_momenta, 1 / moment_of_inertia_of_a_sphere);
    particle->orientation = vector3d__add(particle->orientation, vector3d__scale(change_in_orientation, sample_period));
}

static vector3d_t resultant_force_from_fields(particle_t **particles, const size_t particle_count, const size_t this)
{
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

    return F_resultant;
}

/**
 * Simple 2-body elastic collision for linear momentum
 * 
 *       (m1 - m2)          2 * m2
 * V1f = --------- * V1i + --------- * V2i
 *       (m1 + m2)         (m1 + m2)
 * 
 *        2 * m1           (m2 - m1)
 * V2f = --------- * V1i + --------- * V2i
 *       (m1 + m2)         (m1 + m2)
 */
static void elastic_collision_linear_momenta_update(particle_t *this, particle_t *that)
{
    const vector3d_t Vi_this = vector3d__scale(this->momenta, 1 / this->mass);
    const vector3d_t Vi_that = vector3d__scale(that->momenta, 1 / that->mass);

    const double total_mass = this->mass + that->mass;
    const double mass_diff = this->mass - that->mass;

    const vector3d_t Vf_this = vector3d__add(
        vector3d__scale(Vi_this, mass_diff/total_mass),
        vector3d__scale(Vi_that, 2*that->mass/total_mass)
    );

    const vector3d_t Vf_that = vector3d__add(
        vector3d__scale(Vi_this, 2*this->mass/total_mass),
        vector3d__scale(Vi_that, -mass_diff/total_mass)
    );

    this->momenta = vector3d__scale(Vf_this, this->mass);
    that->momenta = vector3d__scale(Vf_that, that->mass);
}

/**
 * NOTE: Angular momentum is not being conserved along with linear momentum
 *       this way.  This is a placeholder.
 */
static void update_angular_momenta_after_collision(particle_t *this, particle_t *that)
{
    const vector3d_t this_to_that_distance = vector3d__sub(that->pos, this->pos);
    const vector3d_t that_to_this_distance = vector3d__scale(this_to_that_distance, -1);
    const vector3d_t r_this_to_that = vector3d__scale(this_to_that_distance, this->radius / vector3d__mag(this_to_that_distance));
    const vector3d_t r_that_to_this = vector3d__scale(that_to_this_distance, that->radius / vector3d__mag(that_to_this_distance));

    this->angular_momenta = vector3d__cross_product(r_that_to_this, that->momenta);
    that->angular_momenta = vector3d__cross_product(r_this_to_that, this->momenta);
}
