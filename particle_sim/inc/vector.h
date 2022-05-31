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


double vector2d__mag(const vector2d_t a);
double vector2d__distance(const vector2d_t a, const vector2d_t b);
vector2d_t vector2d__add(const vector2d_t a, const vector2d_t b);
vector2d_t vector2d__sub(const vector2d_t a, const vector2d_t b);
vector2d_t vector2d__scale(const vector2d_t a, const double scalar);
double vector2d__dot_product(const vector2d_t a, const vector2d_t b);
double vector2d__theta(const vector2d_t a, const vector2d_t b);

double vector3d__mag(const vector3d_t a);
double vector3d__distance(const vector3d_t a, const vector3d_t b);
vector3d_t vector3d__add(const vector3d_t a, const vector3d_t b);
vector3d_t vector3d__sub(const vector3d_t a, const vector3d_t b);
vector3d_t vector3d__scale(const vector3d_t a, const double scalar);
double vector3d__dot_product(const vector3d_t a, const vector3d_t b);
double vector3d__theta(const vector3d_t a, const vector3d_t b);
vector3d_t vector3d__cross_product(const vector3d_t a, const vector3d_t b);
