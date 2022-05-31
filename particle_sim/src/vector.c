#include "vector.h"

#include <math.h>


double vector_2d__mag(const vector2d_t a)
{
    return sqrt(a.i*a.i + a.j*a.j);
}

double vector_2d__distance(const vector2d_t a, const vector2d_t b)
{
    const double i = a.i - b.i;
    const double j = a.j - b.j;

    return sqrt(i*i + j*j);
}

vector2d_t vector_2d__add(const vector2d_t a, const vector2d_t b)
{
    vector2d_t c;

    c.i = a.i + b.i;
    c.j = a.j + b.j;

    return c;
}

vector2d_t vector_2d__sub(const vector2d_t a, const vector2d_t b)
{
    vector2d_t c;

    c.i = a.i - b.i;
    c.j = a.j - b.j;

    return c;
}

vector2d_t vector_2d__scale(const vector2d_t a, const double scalar)
{
    vector2d_t result = {.i = scalar * a.i, .j = scalar * a.j};
    return result;
}

double vector_2d__dot_product(const vector2d_t a, const vector2d_t b)
{
    return a.i*b.i + a.j*b.j;
}

double vector_2d__theta(const vector2d_t a, const vector2d_t b)
{
    return acos(vector_2d__dot_product(a, b) / (vector_2d__mag(a) * vector_2d__mag(b)));
}

double vector_3d__mag(const vector3d_t a)
{
    return sqrt(a.i*a.i + a.j*a.j + a.k*a.k);
}

double vector_3d__distance(const vector3d_t a, const vector3d_t b)
{
    const double i = a.i - b.i;
    const double j = a.j - b.j;
    const double k = a.k - b.k;

    return sqrt(i*i + j*j + k*k);
}

vector3d_t vector_3d__add(const vector3d_t a, const vector3d_t b)
{
    vector3d_t c;

    c.i = a.i + b.i;
    c.j = a.j + b.j;
    c.k = a.k + b.k;

    return c;
}

vector3d_t vector_3d__sub(const vector3d_t a, const vector3d_t b)
{
    vector3d_t c;

    c.i = a.i - b.i;
    c.j = a.j - b.j;
    c.k = a.k - b.k;

    return c;
}

vector3d_t vector_3d__scale(const vector3d_t a, const double scalar)
{
    vector3d_t result = {.i = scalar * a.i, .j = scalar * a.j, .k = scalar * a.k};
    return result;
}

double vector_3d__dot_product(const vector3d_t a, const vector3d_t b)
{
    return a.i*b.i + a.j*b.j + a.k*b.k;
}

double vector_3d__theta(const vector3d_t a, const vector3d_t b)
{
    return acos(vector_3d__dot_product(a, b) / (vector_3d__mag(a) * vector_3d__mag(b)));
}

vector3d_t vector_3d__cross_product(const vector3d_t a, const vector3d_t b)
{
    vector3d_t c;

    c.i = a.j*b.k - a.k*b.j;
    c.j = a.k*b.i - a.i*b.k;
    c.k = a.i*b.j - a.j*b.i;

    return c;
}
