#pragma once


typedef struct {

    double i;
    double j;
    double k;

} vector_3d_t;

double vector_3d__distance(const vector_3d_t a, const vector_3d_t b);
vector_3d_t vector_3d__add(const vector_3d_t a, const vector_3d_t b);
vector_3d_t vector_3d__sub(const vector_3d_t a, const vector_3d_t b);
double vector_3d__dot_product(const vector_3d_t a, const vector_3d_t b);
vector_3d_t vector_3d__cross_product(const vector_3d_t a, const vector_3d_t b);
