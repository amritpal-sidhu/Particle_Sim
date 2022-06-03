#include "mechanics.h"

#include <math.h>

#include "log.h"


#define __USE_GRAVITY

#define UNIVERSAL_GRAVITY_CONST     6.6743E-17 // (N*m^2)/(g^2)
#define COULOMB_CONST               8.9875E9  // (N*m^2)/(C^2)


static int is_float_negative(const double val);
/* Way to deal with [0, PI] range of acos from vector3d__theta() */
static void correct_signs(vector3d_t *F, const vector3d_t a, const vector3d_t b, const int attract);


extern log_t *log_handle;


/* Public function definitions */
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

void update_positions(particle_t **particles, const size_t particle_count, const double sample_period)
{
    const size_t array_size = P_COUNT+E_COUNT - 1;

    for (size_t this = 0; this < P_COUNT+E_COUNT; ++this) {

        vector3d_t Fe[P_COUNT+E_COUNT-1];
        #ifdef __USE_GRAVITY
        vector3d_t Fg[P_COUNT+E_COUNT-1];
        #endif
        vector3d_t F_resultant = {0};

        /* Try to find a time improvement to compute all forces acting on current particle */
        for (size_t that = 0; that < P_COUNT+E_COUNT; ++that) {

            if (particles[this]->id == particles[that]->id) continue;

            const double r = vector3d__distance(particles[this]->pos, particles[that]->pos);
            const double Fe_mag = electric_force(particles[this]->charge, particles[that]->charge, r);
            #ifdef __USE_GRAVITY
            const double Fg_mag = gravitational_force(particles[this]->mass, particles[that]->mass, r);
            #endif
            const double theta = vector3d__theta(particles[this]->pos, particles[that]->pos);

            const int attract = is_float_negative(particles[this]->charge) != is_float_negative(particles[that]->charge);

            Fe[that] = (vector3d_t){.i = Fe_mag*cos(theta), .j = Fe_mag*sin(theta)};
            correct_signs(&Fe[that], particles[this]->pos, particles[that]->pos, attract);

            #ifdef __USE_GRAVITY
            Fg[that] = (vector3d_t){.i = Fg_mag*cos(theta), .j = Fg_mag*sin(theta)};
            correct_signs(&Fe[that], particles[this]->pos, particles[that]->pos, 1);
            #endif
        }

        for (size_t that = 0; that < P_COUNT+E_COUNT-1; ++that) {
            F_resultant = vector3d__add(F_resultant, Fe[that]);
            #ifdef __USE_GRAVITY
            F_resultant = vector3d__add(F_resultant, Fg[that]);
            #endif
        }

        update_momentum(&particles[this]->momentum_integral, F_resultant, sample_period);
        const vector3d_t change_in_velocity = vector3d__scale(particles[this]->momentum_integral, 1 / particles[this]->mass);

        particles[this]->pos.i += sample_period * change_in_velocity.i;
        particles[this]->pos.j += sample_period * change_in_velocity.j;

        log__write(log_handle, STATUS, "%i,%E,%E,%E,%.2f, %.2f, %.2f",
        particles[this]->id, particles[this]->mass, particles[this]->charge, particles[this]->momentum_integral, particles[this]->pos.i, particles[this]->pos.j, particles[this]->pos.k);
    }

     log__write(log_handle, NONE, "");
}

/* Private function definitions */
static int is_float_negative(const double val)
{
    const unsigned long long int shift = 8*sizeof(double) - 1;
    const unsigned long long int one = 1;
    const unsigned long long int int_val = val;

    return (int_val & (one << shift)) >> shift;
}

static void correct_signs(vector3d_t *F, const vector3d_t a, const vector3d_t b, const int attract)
{
    const vector3d_t F_dir = attract ? vector3d__sub(b, a) : vector3d__sub(a, b);

    if ((F->i > 0 && F_dir.i < 0) || (F->i < 0 && F_dir.i > 0)) F->i *= -1;
    if ((F->j > 0 && F_dir.j < 0) || (F->j < 0 && F_dir.j > 0)) F->j *= -1;
    if ((F->k > 0 && F_dir.k < 0) || (F->k < 0 && F_dir.k > 0)) F->k *= -1;
}
