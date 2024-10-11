#version 460 core


// #define __USE_GRAVITY

#define PROTON_CHARGE       1.602E-19f  // Coulombs
#define ELECTRON_CHARGE     -1.602E-19f

#define PROTON_MASS         1.6727E-24f // Grams
#define NEUTRON_MASS        1.675E-24f
#define ELECTRON_MASS       9.11E-28f

/* Based on Wikipedia covalent radi */
#define HELIUM_NUCLEUS_RADIUS   28E-12f     // Meters
#define ELECTRON_RADIUS         10E-15f     // Meters
#define FAKE_NUCLEUS_RADIUS     0.1f

#define LOCAL_EPSILON               1E-38f

#define UNIVERSAL_GRAVITY_CONST     6.6743E-17f // (N*m^2)/(g^2)
#define COULOMB_CONST               8.9875E9f  // (N*m^2)/(C^2)

struct glsl_particle_t
{
    uint id;
    vec3 pos;
    vec3 momentum;
    vec3 orientation;
    vec3 angular_momentum;
    float mass;
    float charge;
    float radius;
};

/* variables */
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 0) buffer particle_data_block
{
    glsl_particle_t data[];    
};
layout(location = 0) uniform uint id;
layout(location = 1) uniform float sample_period;


/* Local function definitions */
float gravitational_force(const float m1, const float m2, float r)
{
    if (r < LOCAL_EPSILON) r = LOCAL_EPSILON;

    return (UNIVERSAL_GRAVITY_CONST * m1 * m2) / (r * r);
}

float electric_force(const float q1, const float q2, float r)
{
    if (r < LOCAL_EPSILON) r = LOCAL_EPSILON;

    return (COULOMB_CONST * q1 * q2) / (r * r);
}

vec2 componentize_force_2d(const float F, const vec2 direction_vector)
{
    const float azimuth_cos = acos(direction_vector.x / length(direction_vector));
    const float azimuth_sin = asin(direction_vector.y / length(direction_vector));

    return vec2(F*cos(azimuth_cos), F*sin(azimuth_sin));
}

vec3 componentize_force_3d(const float F, const vec3 direction_vector)
{
    const float magnitude = length(direction_vector);
    const float polar_cos = acos(direction_vector.z / magnitude);
    const float polar_sin = asin(distance(direction_vector.x, direction_vector.y) / magnitude);
    const float azimuth_cos = acos(direction_vector.x / (magnitude * sin(polar_sin)));
    const float azimuth_sin = asin(direction_vector.y / (magnitude * sin(polar_sin)));

    return vec3(F*cos(azimuth_cos)*sin(polar_sin), F*sin(azimuth_sin)*sin(polar_sin), F*cos(polar_cos));
}

bool detect_collision(const glsl_particle_t this_part, const glsl_particle_t that)
{
    return distance(this_part.pos, that.pos) < (this_part.radius + that.radius);
}

void update_momentum(const vec3 F)
{
    data[id].momentum += (F * sample_period);
}

void update_position()
{
    const vec3 change_in_velocity = data[id].momentum * (1 / data[id].mass);
    data[id].pos += (change_in_velocity * sample_period);
}

void update_orientation()
{
    /**
     * Moment of inertia of a sphere about its axis is 4/5 M R^2
     * with respect to its surface is 7/5 M R^2
     */
    const float moment_of_inertia_of_a_sphere = 1.4 * data[id].mass * data[id].radius * data[id].radius;
    const vec3 change_in_orientation = data[id].angular_momentum * (1 / moment_of_inertia_of_a_sphere);
    data[id].orientation *= (change_in_orientation * sample_period);
}

vec3 resultant_force_from_fields()
{
    vec3 F_resultant = vec3(0,0,0);

    /* Try to find a time improvement to compute all forces acting on current particle */
    for (uint that = 0; that < data.length(); ++that) {

        if (data[id].id == data[that].id) continue;

        const float r = distance(data[id].pos, data[that].pos);

        F_resultant += componentize_force_3d(
                        electric_force(data[id].charge, data[that].charge, r),
                                      (data[id].pos - data[that].pos));

        #ifdef __USE_GRAVITY
        F_resultant +=  componentize_force_3d(
                            gravitational_force(data[id].mass, data[that].mass, r),
                                               (data[that].pos - data[id].pos));
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
void elastic_collision_linear_momentum_update(glsl_particle_t this_part, glsl_particle_t that)
{
    const vec3 Vi_this = this_part.momentum * (1 / this_part.mass);
    const vec3 Vi_that = that.momentum * (1 / that.mass);

    const float total_mass = this_part.mass + that.mass;
    const float mass_diff = this_part.mass - that.mass;

    const vec3 Vf_this = (Vi_this * mass_diff/total_mass) + (Vi_that * 2*that.mass/total_mass);

    const vec3 Vf_that = (Vi_this * 2*this_part.mass/total_mass) + (Vi_that * -mass_diff/total_mass);

    /**
     * TODO: Test if writing to local variable to function writes back to
     *       variable passed to the function in GLSL.
     *       I doubt it does.
     */
    this_part.momentum = Vf_this * this_part.mass;
    that.momentum = Vf_that * that.mass;
}

/**
 * NOTE: Angular momentum is not being conserved along with linear momentum
 *       this way.  This is a placeholder.
 */
void update_angular_momentum_after_collision(glsl_particle_t this_part, glsl_particle_t that)
{
    const vec3 this_to_that_distance = that.pos - this_part.pos;
    const vec3 that_to_this_distance = -1 * this_to_that_distance;
    const vec3 r_this_to_that = this_to_that_distance * this_part.radius / length(this_to_that_distance);
    const vec3 r_that_to_this = that_to_this_distance * that.radius / length(that_to_this_distance);

    this_part.angular_momentum = cross(r_that_to_this, that.momentum);
    that.angular_momentum = cross(r_this_to_that, this_part.momentum);
}


void time_evolution()
{
    update_momentum(resultant_force_from_fields());
    update_position();
    update_orientation();

    /* Simple check for collision with another particle and perform momentum update */
    for (uint that = 0; that < data.length(); ++that) {
        
        if (data[id].id == data[that].id) continue;

        if (detect_collision(data[id], data[that])) {

            /* Unconserved angular momentum portion */
            update_angular_momentum_after_collision(data[id], data[that]);
            update_orientation();

            elastic_collision_linear_momentum_update(data[id], data[that]);
            update_position();
        }
    }
}


/* main function */
void main()
{
    time_evolution();
}
