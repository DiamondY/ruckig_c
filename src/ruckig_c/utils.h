#ifndef RUCKIG_C_UTILS_H
#define RUCKIG_C_UTILS_H

double ruckig_pow2(double value);
void ruckig_integrate(
    double t,
    double p0,
    double v0,
    double a0,
    double j,
    double* position,
    double* velocity,
    double* acceleration
);

#endif
