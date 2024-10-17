#version 460 core


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


layout(std430, binding = 0) buffer particle_data_block
{
    particle_t particles[];
};
layout(binding = 1) uniform view_data_block
{
    float view_scalar;
    float view_ratio;
};

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) uniform uint index;

out VS_OUT
{
    vec3 color;
} vs_out;


/* Local function prototypes */
mat4 mat4_identity();
mat4 mat4_translate(const vector3d_t pos);
mat4 mat4_rotate_X(const mat4 M, const float angle);
mat4 mat4_rotate_Y(const mat4 M, const float angle);
mat4 mat4_rotate_Z(const mat4 M, const float angle);
mat4 mat4_ortho(const float l, const float r, const float b, const float t, const float n, const float f);


void main()
{
    mat4 mvp, m, p;

    m = mat4_translate(particles[index].pos);
    /* Angles need to be "unscaled" */
    m = mat4_rotate_X(m, particles[index].orientation.x/view_scalar);
    m = mat4_rotate_Y(m, particles[index].orientation.y/view_scalar);
    m = mat4_rotate_Z(m, particles[index].orientation.z/view_scalar);
    m *= view_scalar;

    p = mat4_ortho(-view_ratio, view_ratio, -1, 1, 1, -1);
    mvp = p * m;

    gl_Position = mvp * vec4(in_pos, 1);
    vs_out.color = in_color;
}

/* Local function definitions */
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

    return R * M;
}

mat4 mat4_rotate_Y(const mat4 M, const float angle)
{
    mat4 R;
    const float s = sin(angle);
    const float c = cos(angle);

    R[0].xyzw = vec4(c, 0, -s, 0);
    R[1].xyzw = vec4(0, 1, 0, 0);
    R[2].xyzw = vec4(s, 0, c, 0);
    R[3].xyzw = vec4(0, 0, 0, 1);

    return R * M;
}

mat4 mat4_rotate_Z(const mat4 M, const float angle)
{
    mat4 R;
    const float s = sin(angle);
    const float c = cos(angle);

    R[0].xyzw = vec4(c, s, 0, 0);
    R[1].xyzw = vec4(-s, c, 0, 0);
    R[2].xyzw = vec4(0, 0, 1, 0);
    R[3].xyzw = vec4(0, 0, 0, 1);

    return R * M;
}

mat4 mat4_ortho(const float l, const float r, const float b, const float t, const float n, const float f)
{
    mat4 R;
    R[0][0] = 2/(r-l);
    R[0][1] = R[0][2] = R[0][3] = 0;

	R[1][1] = 2/(t-b);
	R[1][0] = R[1][2] = R[1][3] = 0;

	R[2][2] = -2/(f-n);
	R[2][0] = R[2][1] = R[2][3] = 0;

	R[3][0] = -(r+l)/(r-l);
	R[3][1] = -(t+b)/(t-b);
	R[3][2] = -(f+n)/(f-n);
	R[3][3] = 1;

    return R;
}
