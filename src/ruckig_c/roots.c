#include "ruckig_c/roots.h"

#include <float.h>
#include <math.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

static void sort_values(double* values, size_t count) {
    size_t i;
    for (i = 1; i < count; ++i) {
        double value = values[i];
        size_t j = i;
        while (j > 0 && values[j - 1] > value) {
            values[j] = values[j - 1];
            --j;
        }
        values[j] = value;
    }
}

static void root_set3_insert(ruckig_root_set3_t* roots, double value) {
    if (value >= 0.0 && roots->count < 3) {
        roots->values[roots->count] = value;
        ++roots->count;
    }
}

static void root_set4_insert(ruckig_root_set4_t* roots, double value) {
    if (value >= 0.0 && roots->count < 4) {
        roots->values[roots->count] = value;
        ++roots->count;
    }
}

ruckig_root_set3_t ruckig_solve_cubic(double a, double b, double c, double d) {
    ruckig_root_set3_t roots = {{0.0, 0.0, 0.0}, 0};

    if (fabs(d) < DBL_EPSILON) {
        root_set3_insert(&roots, 0.0);
        d = c;
        c = b;
        b = a;
        a = 0.0;
    }

    if (fabs(a) < DBL_EPSILON) {
        if (fabs(b) < DBL_EPSILON) {
            if (fabs(c) > DBL_EPSILON) {
                root_set3_insert(&roots, -d / c);
            }
        } else {
            const double discriminant = c * c - 4.0 * b * d;
            if (discriminant >= 0.0) {
                const double inv2b = 1.0 / (2.0 * b);
                const double y = sqrt(discriminant);
                root_set3_insert(&roots, (-c + y) * inv2b);
                root_set3_insert(&roots, (-c - y) * inv2b);
            }
        }
    } else {
        const double inva = 1.0 / a;
        const double invaa = inva * inva;
        const double bb = b * b;
        const double bover3a = b * inva / 3.0;
        const double p = (a * c - bb / 3.0) * invaa;
        const double halfq = (2.0 * bb * b - 9.0 * a * b * c + 27.0 * a * a * d) / 54.0 * invaa * inva;
        const double yy = p * p * p / 27.0 + halfq * halfq;

        const double cos120 = -0.50;
        const double sin120 = 0.866025403784438646764;

        if (yy > DBL_EPSILON) {
            const double y = sqrt(yy);
            const double uuu = -halfq + y;
            const double vvv = -halfq - y;
            const double www = fabs(uuu) > fabs(vvv) ? uuu : vvv;
            const double w = cbrt(www);
            root_set3_insert(&roots, w - p / (3.0 * w) - bover3a);
        } else if (yy < -DBL_EPSILON) {
            const double x = -halfq;
            const double y = sqrt(-yy);
            double theta;
            double r;

            if (fabs(x) > DBL_EPSILON) {
                theta = (x > 0.0) ? atan(y / x) : (atan(y / x) + M_PI);
                r = sqrt(x * x - yy);
            } else {
                theta = M_PI / 2.0;
                r = y;
            }

            theta /= 3.0;
            r = 2.0 * cbrt(r);

            {
                const double ux = cos(theta) * r;
                const double uyi = sin(theta) * r;
                root_set3_insert(&roots, ux - bover3a);
                root_set3_insert(&roots, ux * cos120 - uyi * sin120 - bover3a);
                root_set3_insert(&roots, ux * cos120 + uyi * sin120 - bover3a);
            }
        } else {
            const double www = -halfq;
            const double w = 2.0 * cbrt(www);
            root_set3_insert(&roots, w - bover3a);
            root_set3_insert(&roots, w * cos120 - bover3a);
        }
    }

    sort_values(roots.values, roots.count);
    return roots;
}

int ruckig_solve_resolvent(double x[3], double a, double b, double c) {
    const double cos120 = -0.50;
    const double sin120 = 0.866025403784438646764;

    a /= 3.0;
    {
        const double a2 = a * a;
        double q = a2 - b / 3.0;
        const double r = (a * (2.0 * a2 - b) + c) / 2.0;
        const double r2 = r * r;
        const double q3 = q * q * q;

        if (r2 < q3) {
            const double qsqrt = sqrt(q);
            double t = r / (q * qsqrt);
            double theta;
            double ux;
            double uyi;
            if (t < -1.0) {
                t = -1.0;
            } else if (t > 1.0) {
                t = 1.0;
            }
            q = -2.0 * qsqrt;

            theta = acos(t) / 3.0;
            ux = cos(theta) * q;
            uyi = sin(theta) * q;
            x[0] = ux - a;
            x[1] = ux * cos120 - uyi * sin120 - a;
            x[2] = ux * cos120 + uyi * sin120 - a;
            return 3;
        } else {
            double A = -cbrt(fabs(r) + sqrt(r2 - q3));
            double B;
            if (r < 0.0) {
                A = -A;
            }
            B = (0.0 == A ? 0.0 : q / A);

            x[0] = (A + B) - a;
            x[1] = -(A + B) / 2.0 - a;
            x[2] = sqrt(3.0) * (A - B) / 2.0;
            if (fabs(x[2]) < DBL_EPSILON) {
                x[2] = x[1];
                return 2;
            }

            return 1;
        }
    }
}

ruckig_root_set4_t ruckig_solve_quart_monic(double a, double b, double c, double d) {
    ruckig_root_set4_t roots = {{0.0, 0.0, 0.0, 0.0}, 0};

    if (fabs(d) < DBL_EPSILON) {
        if (fabs(c) < DBL_EPSILON) {
            const double D = a * a - 4.0 * b;
            root_set4_insert(&roots, 0.0);
            if (fabs(D) < DBL_EPSILON) {
                root_set4_insert(&roots, -a / 2.0);
            } else if (D > 0.0) {
                const double sqrtD = sqrt(D);
                root_set4_insert(&roots, (-a - sqrtD) / 2.0);
                root_set4_insert(&roots, (-a + sqrtD) / 2.0);
            }
            sort_values(roots.values, roots.count);
            return roots;
        }

        if (fabs(a) < DBL_EPSILON && fabs(b) < DBL_EPSILON) {
            root_set4_insert(&roots, 0.0);
            root_set4_insert(&roots, -cbrt(c));
            sort_values(roots.values, roots.count);
            return roots;
        }
    }

    {
        const double a3 = -b;
        const double b3 = a * c - 4.0 * d;
        const double c3 = -a * a * d - c * c + 4.0 * b * d;
        double x3[3] = {0.0, 0.0, 0.0};
        const int number_zeroes = ruckig_solve_resolvent(x3, a3, b3, c3);
        double y = x3[0];
        double q1;
        double q2;
        double p1;
        double p2;
        double D;
        const double eps = 16.0 * DBL_EPSILON;

        if (number_zeroes != 1) {
            if (fabs(x3[1]) > fabs(y)) {
                y = x3[1];
            }
            if (fabs(x3[2]) > fabs(y)) {
                y = x3[2];
            }
        }

        D = y * y - 4.0 * d;
        if (fabs(D) < DBL_EPSILON) {
            q1 = y / 2.0;
            q2 = y / 2.0;
            D = a * a - 4.0 * (b - y);
            if (fabs(D) < DBL_EPSILON) {
                p1 = a / 2.0;
                p2 = a / 2.0;
            } else {
                const double sqrtD = sqrt(D);
                p1 = (a + sqrtD) / 2.0;
                p2 = (a - sqrtD) / 2.0;
            }
        } else {
            const double sqrtD = sqrt(D);
            q1 = (y + sqrtD) / 2.0;
            q2 = (y - sqrtD) / 2.0;
            p1 = (a * q1 - c) / (q1 - q2);
            p2 = (c - a * q2) / (q1 - q2);
        }

        D = p1 * p1 - 4.0 * q1;
        if (fabs(D) < eps) {
            root_set4_insert(&roots, -p1 / 2.0);
        } else if (D > 0.0) {
            const double sqrtD = sqrt(D);
            root_set4_insert(&roots, (-p1 - sqrtD) / 2.0);
            root_set4_insert(&roots, (-p1 + sqrtD) / 2.0);
        }

        D = p2 * p2 - 4.0 * q2;
        if (fabs(D) < eps) {
            root_set4_insert(&roots, -p2 / 2.0);
        } else if (D > 0.0) {
            const double sqrtD = sqrt(D);
            root_set4_insert(&roots, (-p2 - sqrtD) / 2.0);
            root_set4_insert(&roots, (-p2 + sqrtD) / 2.0);
        }
    }

    sort_values(roots.values, roots.count);
    return roots;
}

double ruckig_poly_eval(const double* p, size_t count, double x) {
    double ret = 0.0;
    size_t k;
    if (!p || count == 0) {
        return ret;
    }

    if (fabs(x) < DBL_EPSILON) {
        ret = p[count - 1];
    } else if (x == 1.0) {
        for (k = count; k > 0; --k) {
            ret += p[k - 1];
        }
    } else {
        double xn = 1.0;
        for (k = count; k > 0; --k) {
            ret += p[k - 1] * xn;
            xn *= x;
        }
    }

    return ret;
}

void ruckig_poly_derivative(const double* coeffs, size_t count, double* derivative) {
    size_t i;
    if (!coeffs || !derivative || count < 2) {
        return;
    }
    for (i = 0; i < count - 1; ++i) {
        derivative[i] = (double)(count - 1 - i) * coeffs[i];
    }
}

void ruckig_poly_monic_derivative(const double* monic_coeffs, size_t count, double* derivative) {
    size_t i;
    if (!monic_coeffs || !derivative || count < 2) {
        return;
    }
    derivative[0] = 1.0;
    for (i = 1; i < count - 1; ++i) {
        derivative[i] = (double)(count - 1 - i) * monic_coeffs[i] / (double)(count - 1);
    }
}

double ruckig_shrink_interval(const double* p, size_t count, double l, double h) {
    const double tolerance = 1e-14;
    const size_t max_its = 128;
    double deriv[16];
    double fl;
    double fh;
    double rts;
    double dxold;
    double dx;
    double f;
    double df;
    size_t j;

    if (!p || count < 2 || count > 17) {
        return NAN;
    }

    fl = ruckig_poly_eval(p, count, l);
    fh = ruckig_poly_eval(p, count, h);
    if (fl == 0.0) {
        return l;
    }
    if (fh == 0.0) {
        return h;
    }
    if (fl > 0.0) {
        const double temp = l;
        l = h;
        h = temp;
    }

    rts = (l + h) / 2.0;
    dxold = fabs(h - l);
    dx = dxold;
    ruckig_poly_derivative(p, count, deriv);
    f = ruckig_poly_eval(p, count, rts);
    df = ruckig_poly_eval(deriv, count - 1, rts);

    for (j = 0; j < max_its; ++j) {
        double temp;
        if ((((rts - h) * df - f) * ((rts - l) * df - f) > 0.0) || (fabs(2.0 * f) > fabs(dxold * df))) {
            dxold = dx;
            dx = (h - l) / 2.0;
            rts = l + dx;
            if (l == rts) {
                break;
            }
        } else {
            dxold = dx;
            dx = f / df;
            temp = rts;
            rts -= dx;
            if (temp == rts) {
                break;
            }
        }

        if (fabs(dx) < tolerance) {
            break;
        }

        f = ruckig_poly_eval(p, count, rts);
        df = ruckig_poly_eval(deriv, count - 1, rts);
        if (f < 0.0) {
            l = rts;
        } else {
            h = rts;
        }
    }

    return rts;
}
