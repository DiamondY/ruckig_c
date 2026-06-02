#ifndef RUCKIG_C_ROOTS_H
#define RUCKIG_C_ROOTS_H

#include <stddef.h>

typedef struct ruckig_root_set3 {
    double values[3];
    size_t count;
} ruckig_root_set3_t;

typedef struct ruckig_root_set4 {
    double values[4];
    size_t count;
} ruckig_root_set4_t;

ruckig_root_set3_t ruckig_solve_cubic(double a, double b, double c, double d);
int ruckig_solve_resolvent(double x[3], double a, double b, double c);
ruckig_root_set4_t ruckig_solve_quart_monic(double a, double b, double c, double d);
double ruckig_poly_eval(const double* p, size_t count, double x);
void ruckig_poly_derivative(const double* coeffs, size_t count, double* derivative);
void ruckig_poly_monic_derivative(const double* monic_coeffs, size_t count, double* derivative);
double ruckig_shrink_interval(const double* p, size_t count, double l, double h);

#endif
