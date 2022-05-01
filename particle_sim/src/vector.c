#include "vector.h"

#include <math.h>

double vector_3d__distance(const vector_3d_t a, const vector_3d_t b)
{
    const double i = (a.i - b.i);
    const double j = (a.j - b.j);
    const double k = (a.k - b.k);

    return sqrt(i*i + j*j + k*k);
}

vector_3d_t vector_3d__add(const vector_3d_t a, const vector_3d_t b)
{
    vector_3d_t c;

    c.i = a.i + b.i;
    c.j = a.j + b.j;
    c.k = a.k + b.k;

    return c;
}

vector_3d_t vector_3d__sub(const vector_3d_t a, const vector_3d_t b)
{
    vector_3d_t c;

    c.i = a.i - b.i;
    c.j = a.j - b.j;
    c.k = a.k - b.k;

    return c;
}


double vector_3d__dot_product(const vector_3d_t a, const vector_3d_t b)
{
    return (a.i*b.i) + (a.j*b.j) + (a.k*b.k);
}

vector_3d_t vector_3d__cross_product(const vector_3d_t a, const vector_3d_t b)
{
    vector_3d_t c;

    c.i = (a.j*b.k) - (a.k*b.j);
    c.j = (a.k*b.i) - (a.i*b.k);
    c.k = (a.i*b.j) - (a.j*b.i);

    return c;
}
