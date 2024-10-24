#version 460 core


// #define __USE_GRAVITY

#define LOCAL_EPSILON               1E-38f

#define UNIVERSAL_GRAVITY_CONST     6.6743E-17f // (N*m^2)/(g^2)
#define COULOMB_CONST               8.9875E9f  // (N*m^2)/(C^2)

#define PARTICLE_COUNT              3

struct vector3d_t
{
    float x;
    float y;
    float z;
};

struct dvector3d_t
{
    double x;
    double y;
    double z;
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


layout(local_size_x = 1) in;

/* variables */
layout(std430, binding = 0) buffer particle_data_block
{
    particle_t particles[];
};
layout(std430, binding = 1) buffer MVP_data_block
{
    mat4 MVP[];
};
layout(binding = 2) uniform uniform_data_block
{
    float sample_period;
    float view_scalar;
    float ratio;
};

vec3 s_pos[PARTICLE_COUNT];
vec3 s_momentum[PARTICLE_COUNT];
vec3 s_orientation[PARTICLE_COUNT];
vec3 s_angular_momentum[PARTICLE_COUNT];
uint index;

/* Local function prototypes */
/**
 * functions to copy SSBO to local shared variables
 * and back to the SSBO
 */
void copy_ssbo_to_shared_variables();
void copy_shared_variables_to_ssbo();

/**
 * function that uses LOCAL_EPSILON when value is zero
 */
double div_zero(const double value);

/**
 * overloaded GLSL functions for vector3d_t
 */
float distance(const vector3d_t v0, const vector3d_t v1);
float length(const vector3d_t v);

double distance(const dvector3d_t v0, const dvector3d_t v1);
double length(const dvector3d_t v);

/**
 * forcing functions
 */
double gravitational_force(const double m1, const double m2, double r);
double electric_force(const double q1, const double q2, double r);

/**
 * functions to componentize force
 */
dvec2 componentize_force_2d(const double F, const dvec2 direction_vector);
dvec3 componentize_force_3d(const double F, const dvec3 direction_vector);

/**
 * functions to update physical state
 */
bool detect_collision(const uint that_index);
void update_momentum(const dvec3 F);
void update_position();
void update_orientation();
dvec3 resultant_force_from_fields();
void elastic_collision_linear_momentum_update(const uint that_index);
void update_angular_momentum_after_collision(const uint that_index);
void time_evolution();

/* matrix operators to compute MVP */
mat4 mat4_identity();
mat4 mat4_translate(vector3d_t pos);
mat4 mat4_rotate_X(mat4 M, const float angle);
mat4 mat4_rotate_Y(mat4 M, const float angle);
mat4 mat4_rotate_Z(mat4 M, const float angle);
mat4 mat4_ortho(const float l, const float r, const float b, const float t, const float n, const float f);
void update_MVP();


/* main function */
void main()
{
    index = gl_WorkGroupID.x;

    update_MVP();

    copy_ssbo_to_shared_variables();
    time_evolution();
    copy_shared_variables_to_ssbo();
}


/* Local function definitions */

void copy_ssbo_to_shared_variables()
{
    s_pos[index] = vec3(particles[index].pos.x, particles[index].pos.y, particles[index].pos.z);
    s_momentum[index] = vec3(particles[index].momentum.x, particles[index].momentum.y, particles[index].momentum.z);
    s_orientation[index] = vec3(particles[index].orientation.x, particles[index].orientation.y, particles[index].orientation.z);
    s_angular_momentum[index] = vec3(particles[index].angular_momentum.x, particles[index].angular_momentum.y, particles[index].angular_momentum.z);
    memoryBarrierShared();
}

void copy_shared_variables_to_ssbo()
{
    particles[index].pos.x = (s_pos[index].x);
    particles[index].pos.y = (s_pos[index].y);
    particles[index].pos.z = (s_pos[index].z);

    particles[index].momentum.x = (s_momentum[index].x);
    particles[index].momentum.y = (s_momentum[index].y);
    particles[index].momentum.z = (s_momentum[index].z);

    particles[index].orientation.x = (s_orientation[index].x);
    particles[index].orientation.y = (s_orientation[index].y);
    particles[index].orientation.z = (s_orientation[index].z);

    particles[index].angular_momentum.x = (s_angular_momentum[index].x);
    particles[index].angular_momentum.y = (s_angular_momentum[index].y);
    particles[index].angular_momentum.z = (s_angular_momentum[index].z);
    memoryBarrierBuffer();
}

double div_zero(const double value)
{
    return (value<LOCAL_EPSILON?LOCAL_EPSILON:value);
}

/**
 * overloaded GLSL functions for vector3d_t
 */
float distance(const vector3d_t v0, const vector3d_t v1)
{
    const float x = v0.x - v1.x;
    const float y = v0.y - v1.y;
    const float z = v0.z - v1.z;

    return sqrt(abs(x*x) + abs(y*y) + abs(z*z));
}

float length(const vector3d_t v)
{
    return sqrt(abs(v.x*v.x) + abs(v.y*v.y) + abs(v.z*v.z));
}

double distance(const dvector3d_t v0, const dvector3d_t v1)
{
    const double x = v0.x - v1.x;
    const double y = v0.y - v1.y;
    const double z = v0.z - v1.z;

    return sqrt(abs(x*x) + abs(y*y) + abs(z*z));
}

double length(const dvector3d_t v)
{
    return sqrt(abs(v.x*v.x) + abs(v.y*v.y) + abs(v.z*v.z));
}

/**
 * forcing functions
 */
double gravitational_force(const double m1, const double m2, double r)
{
    return (UNIVERSAL_GRAVITY_CONST * m1 * m2) / div_zero(r * r);
}

double electric_force(const double q1, const double q2, double r)
{
    return (COULOMB_CONST * q1 * q2) / div_zero(r * r);
}

/**
 * functions to componentize force
 */
dvec2 componentize_force_2d(const double F, const dvec2 direction_vector)
{
    const float azimuth_cos = degrees(acos(float(direction_vector.x / div_zero(length(direction_vector)))));
    const float azimuth_sin = degrees(asin(float(direction_vector.y / div_zero(length(direction_vector)))));

    return dvec2(F*cos(azimuth_cos), F*sin(azimuth_sin));
}

dvec3 componentize_force_3d(const double F, const dvec3 direction_vector)
{
    const float magnitude = length(float(direction_vector));
    const float polar_cos = degrees(acos(float(direction_vector.z / div_zero(magnitude))));
    const float polar_sin = degrees(asin(float(distance(direction_vector.x, direction_vector.y) / div_zero(magnitude))));
    const float azimuth_cos = degrees(acos(float(direction_vector.x / div_zero(magnitude * sin(polar_sin)))));
    const float azimuth_sin = degrees(asin(float(direction_vector.y / div_zero(magnitude * sin(polar_sin)))));

    return dvec3(F*cos(azimuth_cos)*sin(polar_sin), F*sin(azimuth_sin)*sin(polar_sin), F*cos(polar_cos));
}

/**
 * functions to update physical state
 */
bool detect_collision(const uint that_index)
{
    return distance(s_pos[index], s_pos[that_index]) < (particles[index].radius + particles[that_index].radius);
}

void update_momentum(const dvec3 F)
{
    s_momentum[index] += vec3(sample_period * F);
}

void update_position()
{
    const dvec3 change_in_velocity = s_momentum[index] / (particles[index].mass);
    s_pos[index] += vec3(sample_period * change_in_velocity);
}

void update_orientation()
{
    /**
     * Moment of inertia of a sphere about its axis is 4/5 M R^2
     * with respect to its surface is 7/5 M R^2
     */
    const double moment_of_inertia_of_a_sphere = 1.4 * particles[index].mass * particles[index].radius * particles[index].radius;
    const dvec3 change_in_orientation = (s_angular_momentum[index] / div_zero(moment_of_inertia_of_a_sphere));
    s_orientation[index] += vec3(sample_period * change_in_orientation);
}

dvec3 resultant_force_from_fields()
{
    dvec3 F_resultant = dvec3(0, 0, 0);

    /* Try to find a time improvement to compute all forces acting on current particle */
    for (uint that_index = 0; that_index < particles.length(); ++that_index) {

        if (particles[index].id == particles[that_index].id) continue;

        const double r = distance(s_pos[index], s_pos[that_index]);

        F_resultant += componentize_force_3d(
                        electric_force(double(particles[index].charge), double(particles[that_index].charge), r),
                                      (s_pos[index] - s_pos[that_index]));

        #ifdef __USE_GRAVITY
        F_resultant +=  componentize_force_3d(
                            gravitational_force(particles[index].mass, particles[that_index].mass, r),
                                               (s_pos[index] - s_pos[that_index]));
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
    const dvec3 Vi_this = (s_momentum[index] * (1 / particles[index].mass));
    const dvec3 Vi_that = (s_momentum[that_index] * (1 / particles[that_index].mass));

    const double total_mass = div_zero(particles[index].mass + particles[that_index].mass);
    const double mass_diff = particles[index].mass - particles[that_index].mass;

    const dvec3 Vf_this = (Vi_this * mass_diff/total_mass) + (Vi_that * 2*particles[that_index].mass/total_mass);

    const dvec3 Vf_that = (Vi_this * 2*particles[index].mass/total_mass) + (Vi_that * (-mass_diff)/total_mass);

    s_momentum[index] = vec3(Vf_this * particles[index].mass);
    s_momentum[that_index] = vec3(Vf_that * particles[that_index].mass);
}

/**
 * NOTE: Angular momentum is not being conserved along with linear momentum
 *       this way.  This is a placeholder.
 */
void update_angular_momentum_after_collision(const uint that_index)
{
    const dvec3 this_to_that_distance = s_pos[that_index] - s_pos[index];
    const dvec3 that_to_this_distance = -this_to_that_distance;
    const dvec3 r_this_to_that = this_to_that_distance * particles[index].radius / div_zero(length(this_to_that_distance));
    const dvec3 r_that_to_this = that_to_this_distance * particles[that_index].radius / div_zero(length(that_to_this_distance));

    s_angular_momentum[index] = vec3(cross(r_that_to_this, s_momentum[that_index]));
    s_angular_momentum[that_index] = vec3(cross(r_this_to_that, s_momentum[index]));
}


void time_evolution()
{
    update_momentum(resultant_force_from_fields());
    update_position();
    update_orientation();

    // /* Simple check for collision with another particle and perform momentum update */
    for (uint that_index = 0; that_index < particles.length(); ++that_index) {
        
        if (particles[index].id == particles[that_index].id) continue;

        if (detect_collision(that_index)) {

            /* Unconserved angular momentum portion */
            update_angular_momentum_after_collision(that_index);
            update_orientation();

            elastic_collision_linear_momentum_update(that_index);
            update_position();
        }
    }
}

mat4 mat4_identity()
{
    mat4 M;

    for (uint c = 0; c < 4; ++c)
        for (uint r = 0; r < 4; ++r)
            M[c][r] = c==r?1:0;

    return M;
}
mat4 mat4_translate(vector3d_t pos)
{
    mat4 M = mat4_identity();

    M[3].xyz = vec3(pos.x, pos.y, pos.z);

    return M;
}

mat4 mat4_rotate_X(mat4 M, const float angle)
{
    const float s = sin(angle);
    const float c = cos(angle);
    mat4 R;

    R[0].xyzw = vec4(1,  0, 0, 0);
    R[1].xyzw = vec4(0,  c, s, 0);
    R[2].xyzw = vec4(0, -s, c, 0);
    R[3].xyzw = vec4(0,  0, 0, 1);

    return M * R;
}

mat4 mat4_rotate_Y(mat4 M, const float angle)
{
    const float s = sin(angle);
    const float c = cos(angle);
    mat4 R;

    R[0].xyzw = vec4(c, 0, -s, 0);
    R[1].xyzw = vec4(0, 1,  0, 0);
    R[2].xyzw = vec4(s, 0,  c, 0);
    R[3].xyzw = vec4(0, 0,  0, 1);

    return M * R;
}

mat4 mat4_rotate_Z(mat4 M, const float angle)
{
    const float s = sin(angle);
    const float c = cos(angle);
    mat4 R;

    R[0].xyzw = vec4( c, s, 0, 0);
    R[1].xyzw = vec4(-s, c, 0, 0);
    R[2].xyzw = vec4( 0, 0, 1, 0);
    R[3].xyzw = vec4( 0, 0, 0, 1);

    return M * R;
}

mat4 mat4_ortho(const float l, const float r, const float b, const float t, const float n, const float f)
{
    mat4 M;

    M[0][0] = 2/(r-l);
    M[0][1] = M[0][2] = M[0][3] = 0;

    M[1][1] = 2/(t-b);
    M[1][0] = M[1][2] = M[1][3] = 0;

    M[2][2] = 2/(f-n);
    M[2][0] = M[2][1] = M[2][3] = 0;

    M[3][0] = -(r+l)/(r-l);
    M[3][1] = -(t+b)/(t-b);
    M[3][2] = -(f+n)/(f-n);
    M[3][3] = 1;

    return M;
}

void update_MVP()
{
    mat4 M, V, P;

    for (uint i = 0; i < MVP.length(); ++i) {
        M = mat4_translate(particles[index].pos);
        M = mat4_rotate_X(M, degrees(particles[index].orientation.x/view_scalar));
        M = mat4_rotate_Y(M, degrees(particles[index].orientation.y/view_scalar));
        M = mat4_rotate_Z(M, degrees(particles[index].orientation.z/view_scalar));
        M *= view_scalar;
        P = mat4_ortho(-ratio, ratio, -1, 1, 1, -1);
        MVP[index] = P * M;
    }
}
