#version 460 core


// #define __USE_GRAVITY

#define LOCAL_EPSILON               1E-38f

#define UNIVERSAL_GRAVITY_CONST     6.6743E-17f // (N*m^2)/(g^2)
#define COULOMB_CONST               8.9875E9f  // (N*m^2)/(C^2)

struct vector3d_t
{
    float x;
    float y;
    float z;
};

struct particle_t
{
    uint id;
    vector3d_t pos;
    vector3d_t momentum;
    vector3d_t orientation;
    vector3d_t angular_momentum;
    float mass;
    float charge;
    float radius;
};                              

/* variables */
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(std430, binding = 0) buffer particle_data_block
{
    uint index;
    float sample_period;
    particle_t particles[];
};


/* Local function prototypes */
/**
 * overloaded GLSL functions for vector3d_t
 */
float distance(const vector3d_t v0, const vector3d_t v1);
float length(const vector3d_t v);

/**
 * forcing functions
 */
float gravitational_force(const float m1, const float m2, float r);
float electric_force(const float q1, const float q2, float r);

/**
 * functions to componentize force
 */
vec2 componentize_force_2d(const float F, const vec2 direction_vector);
vec3 componentize_force_3d(const float F, const vec3 direction_vector);

/**
 * functions to update physical state
 */
bool detect_collision(const particle_t this_part, const particle_t that_part);
void update_momentum(const vec3 F);
void update_position();
void update_orientation();
vec3 resultant_force_from_fields();
void elastic_collision_linear_momentum_update(const uint that_index);
void update_angular_momentum_after_collision(const uint that_index);
void time_evolution();


/* main function */
void main()
{
    time_evolution();
}


/* Local function definitions */

/**
 * overloaded GLSL functions for vector3d_t
 */
float distance(const vector3d_t v0, const vector3d_t v1)
{
    const float x = v0.x - v1.x;
    const float y = v0.y - v1.y;
    const float z = v0.z - v1.z;

    return sqrt(x*x + y*y + z*z);
}

float length(const vector3d_t v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

/**
 * forcing functions
 */
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

/**
 * functions to componentize force
 */
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

/**
 * functions to update physical state
 */
bool detect_collision(const particle_t this_part, const particle_t that_part)
{
    return distance(this_part.pos, that_part.pos) < (this_part.radius + that_part.radius);
}

void update_momentum(const vec3 F)
{
    vec3 momentum = vec3(particles[index].momentum.x, particles[index].momentum.y, particles[index].momentum.z);

    momentum += (F * sample_period);

    particles[index].momentum.x = momentum.x;
    particles[index].momentum.y = momentum.y;
    particles[index].momentum.z = momentum.z;
}

void update_position()
{
    vec3 pos = vec3(particles[index].pos.x, particles[index].pos.y, particles[index].pos.z);
    const vec3 momentum = vec3(particles[index].momentum.x, particles[index].momentum.y, particles[index].momentum.z);
    
    const vec3 change_in_velocity = momentum * (1 / particles[index].mass);
    pos += (change_in_velocity * sample_period);

    particles[index].pos.x = pos.x;
    particles[index].pos.y = pos.y;
    particles[index].pos.z = pos.z;
}

void update_orientation()
{
    vec3 orientation = vec3(particles[index].orientation.x, particles[index].orientation.y, particles[index].orientation.z);
    const vec3 angular_momentum = vec3(particles[index].angular_momentum.x, particles[index].angular_momentum.y, particles[index].angular_momentum.z);

    /**
     * Moment of inertia of a sphere about its axis is 4/5 M R^2
     * with respect to its surface is 7/5 M R^2
     */
    const float moment_of_inertia_of_a_sphere = 1.4 * particles[index].mass * particles[index].radius * particles[index].radius;
    const vec3 change_in_orientation = angular_momentum * (1 / moment_of_inertia_of_a_sphere);
    orientation *= (change_in_orientation * sample_period);

    particles[index].orientation.x = orientation.x;
    particles[index].orientation.y = orientation.y;
    particles[index].orientation.z = orientation.z;
}

vec3 resultant_force_from_fields()
{
    vec3 F_resultant;
    const vec3 this_pos = vec3(particles[index].pos.x, particles[index].pos.y, particles[index].pos.z);

    /* Try to find a time improvement to compute all forces acting on current particle */
    for (uint that_index = 0; that_index < particles.length(); ++that_index) {

        if (particles[index].id == particles[that_index].id) continue;

        const vec3 that_pos = vec3(particles[that_index].pos.x, particles[that_index].pos.y, particles[that_index].pos.z);
        const float r = distance(particles[index].pos, particles[that_index].pos);

        F_resultant += componentize_force_3d(
                        electric_force(particles[index].charge, particles[that_index].charge, r),
                                      (this_pos - that_pos));

        #ifdef __USE_GRAVITY
        F_resultant +=  componentize_force_3d(
                            gravitational_force(particles[index].mass, particles[that_index].mass, r),
                                               (this_pos - that_pos));
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
void elastic_collision_linear_momentum_update(const uint that_index)
{
    const vec3 this_momentum = vec3(particles[index].momentum.x, particles[index].momentum.y, particles[index].momentum.z);
    const vec3 that_momentum = vec3(particles[that_index].momentum.x, particles[that_index].momentum.y, particles[that_index].momentum.z);

    const vec3 Vi_this = this_momentum * (1 / particles[index].mass);
    const vec3 Vi_that = that_momentum * (1 / particles[that_index].mass);

    const float total_mass = particles[index].mass + particles[that_index].mass;
    const float mass_diff = particles[index].mass - particles[that_index].mass;

    const vec3 Vf_this = (Vi_this * mass_diff/total_mass) + (Vi_that * 2*particles[that_index].mass/total_mass);

    const vec3 Vf_that = (Vi_this * 2*particles[index].mass/total_mass) + (Vi_that * -mass_diff/total_mass);

    const vec3 this_result_momentum = Vf_this * particles[index].mass;
    const vec3 that_result_momentum = Vf_that * particles[that_index].mass;

    particles[index].momentum.x = this_result_momentum.x;
    particles[index].momentum.y = this_result_momentum.y;
    particles[index].momentum.z = this_result_momentum.z;
    particles[that_index].momentum.x = that_result_momentum.x;
    particles[that_index].momentum.y = that_result_momentum.y;
    particles[that_index].momentum.z = that_result_momentum.z;
}

/**
 * NOTE: Angular momentum is not being conserved along with linear momentum
 *       this way.  This is a placeholder.
 */
void update_angular_momentum_after_collision(const uint that_index)
{
    const vec3 this_pos = vec3(particles[index].pos.x, particles[index].pos.y, particles[index].pos.z);
    const vec3 that_pos = vec3(particles[that_index].pos.x, particles[that_index].pos.y, particles[that_index].pos.z);
    const vec3 this_momentum = vec3(particles[index].momentum.x, particles[index].momentum.y, particles[index].momentum.z);
    const vec3 that_momentum = vec3(particles[that_index].momentum.x, particles[that_index].momentum.y, particles[that_index].momentum.z);

    const vec3 this_to_that_distance = that_pos - this_pos;
    const vec3 that_to_this_distance = -1 * this_to_that_distance;
    const vec3 r_this_to_that = this_to_that_distance * particles[index].radius / length(this_to_that_distance);
    const vec3 r_that_to_this = that_to_this_distance * particles[that_index].radius / length(that_to_this_distance);

    const vec3 this_angular_momentum = cross(r_that_to_this, that_momentum);
    const vec3 that_angular_momentum = cross(r_this_to_that, this_momentum);

    particles[index].angular_momentum.x = this_angular_momentum.x;
    particles[index].angular_momentum.y = this_angular_momentum.y;
    particles[index].angular_momentum.z = this_angular_momentum.z;
    particles[that_index].angular_momentum.x = that_angular_momentum.x;
    particles[that_index].angular_momentum.y = that_angular_momentum.y;
    particles[that_index].angular_momentum.z = that_angular_momentum.z;
}


void time_evolution()
{
    update_momentum(resultant_force_from_fields());
    update_position();
    update_orientation();

    /* Simple check for collision with another particle and perform momentum update */
    for (uint that_index = 0; that_index < particles.length(); ++that_index) {
        
        if (particles[index].id == particles[that_index].id) continue;

        if (detect_collision(particles[index], particles[that_index])) {

            /* Unconserved angular momentum portion */
            update_angular_momentum_after_collision(that_index);
            update_orientation();

            elastic_collision_linear_momentum_update(that_index);
            update_position();
        }
    }
}
