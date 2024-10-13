#version 460 core

struct vector3d_t
{
    float x, y, z;
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


layout(binding = 0) buffer particle_data_block
{
    particle_t particles[];    
};
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) uniform uint id;
layout(location = 3) uniform float view_scalar;
layout(location = 4) uniform float view_ratio;
out vec3 vertex_color;


/* Private function prototypes */
mat4 mat4_identity();
mat4 mat4_translate(const vector3d_t pos);
mat4 mat4_rotate_X(const mat4 M, const float angle);
mat4 mat4_rotate_Y(const mat4 M, const float angle);
mat4 mat4_rotate_Z(const mat4 M, const float angle);
mat4 mat4_ortho(mat4 M, const float l, const float r, const float b, const float t, const float n, const float f);


void main()
{
    mat4 mvp, m, v, p;

    m = mat4_translate(particles[id].pos);
    /* Angles need to be "unscaled" */
    v = mat4_rotate_X(m, particles[id].orientation.x/view_scalar);
    v = mat4_rotate_Y(m, particles[id].orientation.y/view_scalar);
    v = mat4_rotate_Z(m, particles[id].orientation.z/view_scalar);
    v *= view_scalar;

    p = mat4_ortho(p, -view_ratio, view_ratio, -1, 1, 1, -1);
    mvp = p * v * m;

    gl_Position = mvp * vec4(in_pos, 1);
    vertex_color = in_color;
}


/* Private function definitions */
mat4 mat4_identity()
{
    mat4 M;

    for (uint i = 0; i < 4; ++i)
        for (uint j = 0; j < 4; ++j)
            M[i][j] = i==j?1:0;

    return M;
}

mat4 mat4_translate(const vector3d_t pos)
{
    mat4 T;

    T = mat4_identity();
    T[3][0] = pos.x;
    T[3][1] = pos.y;
    T[3][2] = pos.z;

    return T;
}

mat4 mat4_rotate_X(const mat4 M, const float angle)
{
    mat4 R;
    const float s = sin(angle);
    const float c = cos(angle);

    R[0].xyzw = vec4(1,  0,  0, 0);
    R[1].xyzw = vec4(0,  c,  s, 0);
    R[2].xyzw = vec4(0, -s,  c, 0);
    R[3].xyzw = vec4(0,  0,  0, 1);

    return M * R;
}

mat4 mat4_rotate_Y(const mat4 M, const float angle)
{
    mat4 R;
    const float s = sin(angle);
    const float c = cos(angle);

    R[0].xyzw = vec4(c, 0, -s, 0);
    R[1].xyzw = vec4(0, 1,  0, 0);
    R[2].xyzw = vec4(s, 0,  c, 0);
    R[3].xyzw = vec4(0, 0,  0, 1);

    return M * R;
}

mat4 mat4_rotate_Z(const mat4 M, const float angle)
{
    mat4 R;
    const float s = sin(angle);
    const float c = cos(angle);

    R[0].xyzw = vec4(c,  s, 0, 0);
    R[1].xyzw = vec4(-s, c, 0, 0);
    R[2].xyzw = vec4(0,  0, 1, 0);
    R[3].xyzw = vec4(0,  0, 0, 1);

    return M * R;
}

mat4 mat4_ortho(mat4 M, const float l, const float r, const float b, const float t, const float n, const float f)
{
    mat4 R;
    R[0][0] = 2.f/(r-1);
    R[0][1] = M[0][3] = M[0][2] = 0.f;

	R[1][1] = 2.f/(t-b);
	R[1][0] = M[1][3] = M[1][2] = 0.f;

	R[2][2] = -2.f/(f-n);
	R[2][0] = M[2][3] = M[2][1] = 0.f;

	R[3][0] = -(r+l)/(r-l);
	R[3][1] = -(t+b)/(t-b);
	R[3][2] = -(f+n)/(f-n);
	R[3][3] = 1.f;

    return R;
}
