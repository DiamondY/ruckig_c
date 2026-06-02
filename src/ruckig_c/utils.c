#include "ruckig_c/utils.h"

double ruckig_pow2(double value) {
    return value * value;
}

void ruckig_integrate(
    double t,
    double p0,
    double v0,
    double a0,
    double j,
    double* position,
    double* velocity,
    double* acceleration
) {
    if (position) {
        *position = p0 + t * (v0 + t * (a0 / 2.0 + t * j / 6.0));
    }
    if (velocity) {
        *velocity = v0 + t * (a0 + t * j / 2.0);
    }
    if (acceleration) {
        *acceleration = a0 + t * j;
    }
}
