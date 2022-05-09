#pragma once


typedef struct {

    double i;
    double j;

} vector_2d_t;

typedef struct {

    double i;
    double j;
    double k;

} vector_3d_t;

typedef struct {

    float r;
    float g;
    float b;

} color_t;


double vector_2d__mag(const vector_2d_t a);
double vector_2d__distance(const vector_2d_t a, const vector_2d_t b);
vector_2d_t vector_2d__add(const vector_2d_t a, const vector_2d_t b);
vector_2d_t vector_2d__sub(const vector_2d_t a, const vector_2d_t b);
double vector_2d__dot_product(const vector_2d_t a, const vector_2d_t b);

double vector_3d__mag(const vector_3d_t a);
double vector_3d__distance(const vector_3d_t a, const vector_3d_t b);
vector_3d_t vector_3d__add(const vector_3d_t a, const vector_3d_t b);
vector_3d_t vector_3d__sub(const vector_3d_t a, const vector_3d_t b);
double vector_3d__dot_product(const vector_3d_t a, const vector_3d_t b);
vector_3d_t vector_3d__cross_product(const vector_3d_t a, const vector_3d_t b);
