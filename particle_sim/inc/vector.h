#pragma once


typedef struct {

    double i;
    double j;

} vector2d_t;

typedef struct {

    double i;
    double j;
    double k;

} vector3d_t;

typedef struct {

    float r;
    float g;
    float b;

} color_t;


double vector_2d__mag(const vector2d_t a);
double vector_2d__distance(const vector2d_t a, const vector2d_t b);
vector2d_t vector_2d__add(const vector2d_t a, const vector2d_t b);
vector2d_t vector_2d__sub(const vector2d_t a, const vector2d_t b);
vector2d_t vector_2d__scale(const vector2d_t a, const double scalar);
double vector_2d__dot_product(const vector2d_t a, const vector2d_t b);
double vector_2d__theta(const vector2d_t a, const vector2d_t b);

double vector_3d__mag(const vector3d_t a);
double vector_3d__distance(const vector3d_t a, const vector3d_t b);
vector3d_t vector_3d__add(const vector3d_t a, const vector3d_t b);
vector3d_t vector_3d__sub(const vector3d_t a, const vector3d_t b);
vector3d_t vector_3d__scale(const vector3d_t a, const double scalar);
double vector_3d__dot_product(const vector3d_t a, const vector3d_t b);
double vector_3d__theta(const vector3d_t a, const vector3d_t b);
vector3d_t vector_3d__cross_product(const vector3d_t a, const vector3d_t b);
