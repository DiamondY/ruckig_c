#include "ruckig_c/position_first.h"

#include "ruckig_c/precision.h"
#include "ruckig_c/roots.h"
#include "ruckig_c/utils.h"

#include <float.h>
#include <math.h>
#include <string.h>

static void clear_times(ruckig_profile_t* profile) {
    memset(profile->t, 0, sizeof(profile->t));
}

/* Rest-to-rest velocity-limited UDDU family; tf/4 bounds the symmetric four jerk pulses. */
static bool time_vel_rest_to_rest(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    ruckig_root_set3_t roots;
    size_t i;

    if (fabs(v0) >= DBL_EPSILON || fabs(a0) >= DBL_EPSILON || fabs(vf) >= DBL_EPSILON || fabs(af) >= DBL_EPSILON) {
        return false;
    }
    if (fabs(j_max) < DBL_EPSILON) {
        return false;
    }

    roots = ruckig_solve_cubic(2.0 * j_max, -j_max * tf, 0.0, pd);
    for (i = 0; i < roots.count; ++i) {
        const double tj = roots.values[i];
        if (tj <= 0.0 || tj > tf / 4.0) {
            continue;
        }

        clear_times(profile);
        profile->t[0] = tj;
        profile->t[2] = tj;
        profile->t[3] = tf - 4.0 * tj;
        profile->t[4] = tj;
        profile->t[6] = tj;

        if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_VEL, tf, j_max, v_max, v_min, a_max, a_min)) {
            return true;
        }
    }

    return false;
}

/* Refine a candidate UDDU root once, then let the profile timing check validate limits. */
static bool check_time_vel_general_uddu_root(
    ruckig_profile_t* profile,
    double t,
    double tf,
    double vd,
    double v0,
    double a0,
    double af,
    double pd,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double j_max_j_max = j_max * j_max;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double af_p3 = af_af * af;
    double h1;
    double radicand;

    radicand = (a0_a0 + af_af) / (2.0 * j_max_j_max) + (2.0 * a0 * t + j_max * t * t - vd) / j_max;
    if (radicand < 0.0) {
        return false;
    }
    h1 = sqrt(radicand);
    {
        const double orig = -pd
            - (2.0 * a0_p3 + 4.0 * af_p3
                + 24.0 * a0 * j_max * t * (af + j_max * (h1 + t - tf))
                + 6.0 * a0_a0 * (af + j_max * (2.0 * t - tf))
                + 6.0 * (a0_a0 + af_af) * j_max * h1
                + 12.0 * af * j_max * (j_max * t * t - vd)
                + 12.0 * j_max_j_max * (j_max * t * t * (h1 + t - tf) - tf * v0 - h1 * vd))
                / (12.0 * j_max_j_max);
        const double deriv_newton = -(a0 + j_max * t) * (3.0 * (h1 + t) - 2.0 * tf + (a0 + 2.0 * af) / j_max);
        if (!isnan(orig) && !isnan(deriv_newton) && fabs(deriv_newton) > DBL_EPSILON) {
            t -= orig / deriv_newton;
        }
    }

    if (t > tf || isnan(t)) {
        return false;
    }

    radicand = (a0_a0 + af_af) / (2.0 * j_max_j_max) + (t * (2.0 * a0 + j_max * t) - vd) / j_max;
    if (radicand < 0.0) {
        return false;
    }
    h1 = sqrt(radicand);

    clear_times(profile);
    profile->t[0] = t;
    profile->t[2] = t + a0 / j_max;
    profile->t[3] = tf - 2.0 * (t + h1) - (a0 + af) / j_max;
    profile->t[4] = h1;
    profile->t[6] = h1 + af / j_max;

    return ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_VEL, tf, j_max, v_max, v_min, a_max, a_min);
}

static bool time_vel_general_uddu(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double tf_tf = tf * tf;
    const double vd = vf - v0;
    const double vd_vd = vd * vd;
    const double ad = af - a0;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double a0_p6 = a0_p4 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double af_p6 = af_p4 * af_af;
    const double j_max_j_max = j_max * j_max;
    const double g1 = -pd + tf * v0;
    const double tz_min = fmax(0.0, -a0 / j_max);
    const double tz_max = fmin((tf - a0 / j_max) / 2.0, (a_max - a0) / j_max);
    const double p1 = af_af - 2.0 * j_max * (-2.0 * af * tf + j_max * tf_tf + 3.0 * vd);
    const double ph1 = af_p3 - 3.0 * j_max_j_max * g1 - 3.0 * af * j_max * vd;
    const double ph2 = af_p4 + 8.0 * af_p3 * j_max * tf
        + 12.0 * j_max * (3.0 * j_max * vd_vd - af_af * vd + 2.0 * af * j_max * (g1 - tf * vd) - 2.0 * j_max_j_max * tf * g1);
    const double ph3 = a0 * (af - j_max * tf);
    const double ph4 = j_max * (-ad + j_max * tf);
    double polynom[6];
    double deriv[5];
    double dderiv[4];
    ruckig_root_set4_t d_extremas;
    double tz_current = tz_min;
    size_t i;

    if (tz_max <= tz_min || fabs(ph4) < DBL_EPSILON) {
        return false;
    }

    polynom[0] = 1.0;
    polynom[1] = (15.0 * a0_a0 + af_af + 4.0 * af * j_max * tf - 16.0 * ph3 - 2.0 * j_max * (j_max * tf_tf + 3.0 * vd)) / (4.0 * ph4);
    polynom[2] = (29.0 * a0_p3 - 2.0 * af_p3 - 33.0 * a0 * ph3 + 6.0 * j_max_j_max * g1 + 6.0 * af * j_max * vd + 6.0 * a0 * p1) / (6.0 * j_max * ph4);
    polynom[3] = (61.0 * a0_p4 - 76.0 * a0_a0 * ph3 - 16.0 * a0 * ph1 + 30.0 * a0_a0 * p1 + ph2) / (24.0 * j_max_j_max * ph4);
    polynom[4] = (a0 * (7.0 * a0_p4 - 10.0 * a0_a0 * ph3 - 4.0 * a0 * ph1 + 6.0 * a0_a0 * p1 + ph2)) / (12.0 * j_max_j_max * j_max * ph4);
    polynom[5] = (7.0 * a0_p6 + af_p6 - 12.0 * a0_p4 * ph3 + 48.0 * af_p3 * j_max_j_max * g1
            - 8.0 * a0_p3 * ph1 - 72.0 * j_max_j_max * j_max * (j_max * g1 * g1 + vd_vd * vd + 2.0 * af * g1 * vd)
            - 6.0 * af_p4 * j_max * vd + 36.0 * af_af * j_max_j_max * vd_vd + 9.0 * a0_p4 * p1 + 3.0 * a0_a0 * ph2)
        / (144.0 * j_max_j_max * j_max_j_max * ph4);

    ruckig_poly_monic_derivative(polynom, 6, deriv);
    ruckig_poly_derivative(deriv, 5, dderiv);
    d_extremas = ruckig_solve_quart_monic(deriv[1], deriv[2], deriv[3], deriv[4]);

    for (i = 0; i < d_extremas.count; ++i) {
        double tz = d_extremas.values[i];
        double orig;
        double val_new;
        if (tz >= tz_max) {
            continue;
        }

        orig = ruckig_poly_eval(deriv, 5, tz);
        if (fabs(orig) > RUCKIG_C_POLY_ROOT_REFINEMENT_TOLERANCE) {
            const double d2 = ruckig_poly_eval(dderiv, 4, tz);
            if (fabs(d2) > DBL_EPSILON) {
                tz -= orig / d2;
            }
        }

        val_new = ruckig_poly_eval(polynom, 6, tz);
        if (fabs(val_new) < 64.0 * fabs(ruckig_poly_eval(dderiv, 4, tz)) * RUCKIG_C_POLY_ROOT_REFINEMENT_TOLERANCE) {
            if (check_time_vel_general_uddu_root(profile, tz, tf, vd, v0, a0, af, pd, v_max, v_min, a_max, a_min, j_max)) {
                return true;
            }
        } else if (ruckig_poly_eval(polynom, 6, tz_current) * val_new < 0.0) {
            if (check_time_vel_general_uddu_root(profile, ruckig_shrink_interval(polynom, 6, tz_current, tz), tf, vd, v0, a0, af, pd, v_max, v_min, a_max, a_min, j_max)) {
                return true;
            }
        }
        tz_current = tz;
    }

    {
        const double val_max = ruckig_poly_eval(polynom, 6, tz_max);
        if (ruckig_poly_eval(polynom, 6, tz_current) * val_max < 0.0) {
            if (check_time_vel_general_uddu_root(profile, ruckig_shrink_interval(polynom, 6, tz_current, tz_max), tf, vd, v0, a0, af, pd, v_max, v_min, a_max, a_min, j_max)) {
                return true;
            }
        } else if (fabs(val_max) < RUCKIG_C_POLY_BOUNDARY_EPS_FACTOR * DBL_EPSILON) {
            if (check_time_vel_general_uddu_root(profile, tz_max, tf, vd, v0, a0, af, pd, v_max, v_min, a_max, a_min, j_max)) {
                return true;
            }
        }
    }

    return false;
}

static bool check_time_vel_general_udud_root(
    ruckig_profile_t* profile,
    double t,
    double tf,
    double pd,
    double vd,
    double v0,
    double a0,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double ad = af - a0;
    const double j_max_j_max = j_max * j_max;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double af_p3 = af_af * af;
    double h1;
    double radicand;
    double orig;
    double deriv_newton;

    radicand = (af_af - a0_a0) / (2.0 * j_max_j_max) - ((2.0 * a0 + j_max * t) * t - vd) / j_max;
    if (radicand < 0.0) {
        return false;
    }
    h1 = sqrt(radicand);
    orig = -pd + (af_p3 - a0_p3 + 3.0 * a0_a0 * j_max * (tf - 2.0 * t)) / (6.0 * j_max_j_max)
        + (2.0 * a0 + j_max * t) * t * (tf - t)
        + (j_max * h1 - af) * h1 * h1 + tf * v0;
    deriv_newton = (a0 + j_max * t) * (2.0 * (af + j_max * tf) - 3.0 * j_max * (h1 + t) - a0) / j_max;
    if (fabs(deriv_newton) > DBL_EPSILON) {
        t -= orig / deriv_newton;
    }

    radicand = (af_af - a0_a0) / (2.0 * j_max_j_max) - ((2.0 * a0 + j_max * t) * t - vd) / j_max;
    if (radicand < 0.0) {
        return false;
    }
    h1 = sqrt(radicand);
    orig = -pd + (af_p3 - a0_p3 + 3.0 * a0_a0 * j_max * (tf - 2.0 * t)) / (6.0 * j_max_j_max)
        + (2.0 * a0 + j_max * t) * t * (tf - t)
        + (j_max * h1 - af) * h1 * h1 + tf * v0;
    if (fabs(orig) > RUCKIG_C_TIME_SYNC_RESIDUAL_TOLERANCE) {
        deriv_newton = (a0 + j_max * t) * (2.0 * (af + j_max * tf) - 3.0 * j_max * (h1 + t) - a0) / j_max;
        if (fabs(deriv_newton) > DBL_EPSILON) {
            t -= orig / deriv_newton;
        }
    }

    radicand = (af_af - a0_a0) / (2.0 * j_max_j_max) - ((2.0 * a0 + j_max * t) * t - vd) / j_max;
    if (radicand < 0.0 || isnan(t)) {
        return false;
    }
    h1 = sqrt(radicand);

    clear_times(profile);
    profile->t[0] = t;
    profile->t[2] = t + a0 / j_max;
    profile->t[3] = tf - 2.0 * (t + h1) + ad / j_max;
    profile->t[4] = h1;
    profile->t[6] = h1 - af / j_max;

    return ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDUD, RUCKIG_PROFILE_LIMITS_VEL, tf, j_max, v_max, v_min, a_max, a_min);
}

static bool time_vel_general_udud(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double tf_tf = tf * tf;
    const double vd = vf - v0;
    const double vd_vd = vd * vd;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double a0_p5 = a0_p4 * a0;
    const double a0_p6 = a0_p4 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double af_p6 = af_p4 * af_af;
    const double j_max_j_max = j_max * j_max;
    const double g1 = -pd + tf * v0;
    const double tz_min = fmax(0.0, -a0 / j_max);
    const double tz_max = fmin((tf - a0 / j_max) / 2.0, (a_max - a0) / j_max);
    const double ph1 = af_af - 2.0 * j_max * (2.0 * af * tf + j_max * tf_tf - 3.0 * vd);
    const double ph2 = af_p3 - 3.0 * j_max_j_max * g1 + 3.0 * af * j_max * vd;
    const double ph3 = 2.0 * j_max * tf * g1 + 3.0 * vd_vd;
    const double ph4 = af_p4 - 8.0 * af_p3 * j_max * tf
        + 12.0 * j_max * (j_max * ph3 + af_af * vd + 2.0 * af * j_max * (g1 - tf * vd));
    const double ph5 = af + j_max * tf;
    double polynom[7];
    double deriv[6];
    double dderiv[5];
    double ddderiv[4];
    double intervals[6][2];
    size_t interval_count = 0;
    double dd_tz_current = tz_min;
    double tz_current = tz_min;
    ruckig_root_set4_t dd_extremas;
    size_t i;

    if (tz_max <= tz_min || fabs(j_max) < DBL_EPSILON) {
        return false;
    }

    polynom[0] = 1.0;
    polynom[1] = (5.0 * a0 - ph5) / j_max;
    polynom[2] = (39.0 * a0_a0 - ph1 - 16.0 * a0 * ph5) / (4.0 * j_max_j_max);
    polynom[3] = (55.0 * a0_p3 - 33.0 * a0_a0 * ph5 - 6.0 * a0 * ph1 + 2.0 * ph2) / (6.0 * j_max_j_max * j_max);
    polynom[4] = (101.0 * a0_p4 + ph4 - 76.0 * a0_p3 * ph5 - 30.0 * a0_a0 * ph1 + 16.0 * a0 * ph2) / (24.0 * j_max_j_max * j_max_j_max);
    polynom[5] = (a0 * (11.0 * a0_p4 + ph4 - 10.0 * a0_p3 * ph5 - 6.0 * a0_a0 * ph1 + 4.0 * a0 * ph2)) / (12.0 * j_max_j_max * j_max_j_max * j_max);
    polynom[6] = (11.0 * a0_p6 - af_p6 - 12.0 * a0_p5 * ph5 - 48.0 * af_p3 * j_max_j_max * g1
            - 9.0 * a0_p4 * ph1 + 72.0 * j_max_j_max * j_max * (j_max * g1 * g1 - vd_vd * vd - 2.0 * af * g1 * vd)
            - 6.0 * af_p4 * j_max * vd - 36.0 * af_af * j_max_j_max * vd_vd + 8.0 * a0_p3 * ph2 + 3.0 * a0_a0 * ph4)
        / (144.0 * j_max_j_max * j_max_j_max * j_max_j_max);

    ruckig_poly_monic_derivative(polynom, 7, deriv);
    ruckig_poly_monic_derivative(deriv, 6, dderiv);
    ruckig_poly_derivative(dderiv, 5, ddderiv);
    dd_extremas = ruckig_solve_quart_monic(dderiv[1], dderiv[2], dderiv[3], dderiv[4]);

    for (i = 0; i < dd_extremas.count; ++i) {
        double tz = dd_extremas.values[i];
        double orig;
        if (tz >= tz_max) {
            continue;
        }

        orig = ruckig_poly_eval(dderiv, 5, tz);
        if (fabs(orig) > RUCKIG_C_POLY_ROOT_REFINEMENT_TOLERANCE) {
            const double d3 = ruckig_poly_eval(ddderiv, 4, tz);
            if (fabs(d3) > DBL_EPSILON) {
                tz -= orig / d3;
            }
        }

        if (ruckig_poly_eval(deriv, 6, dd_tz_current) * ruckig_poly_eval(deriv, 6, tz) < 0.0 && interval_count < 6) {
            intervals[interval_count][0] = dd_tz_current;
            intervals[interval_count][1] = tz;
            ++interval_count;
        }
        dd_tz_current = tz;
    }
    if (ruckig_poly_eval(deriv, 6, dd_tz_current) * ruckig_poly_eval(deriv, 6, tz_max) < 0.0 && interval_count < 6) {
        intervals[interval_count][0] = dd_tz_current;
        intervals[interval_count][1] = tz_max;
        ++interval_count;
    }

    for (i = 0; i < interval_count; ++i) {
        const double tz = ruckig_shrink_interval(deriv, 6, intervals[i][0], intervals[i][1]);
        double p_val;
        if (tz >= tz_max || isnan(tz)) {
            continue;
        }

        p_val = ruckig_poly_eval(polynom, 7, tz);
        if (fabs(p_val) < 64.0 * fabs(ruckig_poly_eval(dderiv, 5, tz)) * RUCKIG_C_POLY_ROOT_REFINEMENT_TOLERANCE) {
            if (check_time_vel_general_udud_root(profile, tz, tf, pd, vd, v0, a0, af, v_max, v_min, a_max, a_min, j_max)) {
                return true;
            }
        } else if (ruckig_poly_eval(polynom, 7, tz_current) * p_val < 0.0) {
            if (check_time_vel_general_udud_root(profile, ruckig_shrink_interval(polynom, 7, tz_current, tz), tf, pd, vd, v0, a0, af, v_max, v_min, a_max, a_min, j_max)) {
                return true;
            }
        }
        tz_current = tz;
    }

    if (ruckig_poly_eval(polynom, 7, tz_current) * ruckig_poly_eval(polynom, 7, tz_max) < 0.0) {
        if (check_time_vel_general_udud_root(profile, ruckig_shrink_interval(polynom, 7, tz_current, tz_max), tf, pd, vd, v0, a0, af, v_max, v_min, a_max, a_min, j_max)) {
            return true;
        }
    }

    return false;
}

static bool time_acc0_acc1_vel(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double vd = vf - v0;
    const double vd_vd = vd * vd;
    const double ad = af - a0;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;
    const double g1 = -pd + tf * v0;

    if (fabs(j_max) < DBL_EPSILON || fabs(a_max) < DBL_EPSILON || fabs(a_min) < DBL_EPSILON) {
        return false;
    }

    if ((2.0 * (a_max - a_min) + ad) / j_max < tf) {
        const double radicand = (a0_p4 + af_p4
                - 4.0 * a0_p3 * (2.0 * a_max + a_min) / 3.0
                - 4.0 * af_p3 * (a_max + 2.0 * a_min) / 3.0
                + 2.0 * (a0_a0 - af_af) * a_max * a_max
                + (4.0 * a0 * a_max - 2.0 * a0_a0) * (af_af - 2.0 * af * a_min + (a_min - a_max) * a_min + 2.0 * j_max * (a_min * tf - vd))
                + 2.0 * af_af * (a_min * a_min + 2.0 * j_max * (a_max * tf - vd))
                + 4.0 * j_max * (2.0 * a_min * (af * vd + j_max * g1) + (a_max * a_max - a_min * a_min) * vd + j_max * vd_vd)
                + 8.0 * a_max * j_max_j_max * (pd - tf * vf)) / (a_max * a_min)
            + 4.0 * af_af + 2.0 * a0_a0 + (4.0 * af + a_max - a_min) * (a_max - a_min)
            + 4.0 * j_max * (a_min - a_max + j_max * tf - 2.0 * af) * tf;
        double h1;

        if (radicand >= 0.0) {
            h1 = sqrt(radicand) * fabs(j_max) / j_max;

            clear_times(profile);
            profile->t[0] = (-a0 + a_max) / j_max;
            profile->t[1] = (-(af_af - a0_a0 + 2.0 * a_max * a_max + a_min * (a_min - 2.0 * ad - 3.0 * a_max) + 2.0 * j_max * (a_min * tf - vd)) + a_min * h1) / (2.0 * (a_max - a_min) * j_max);
            profile->t[2] = a_max / j_max;
            profile->t[3] = (a_min - a_max + h1) / (2.0 * j_max);
            profile->t[4] = -a_min / j_max;
            profile->t[5] = tf - (profile->t[0] + profile->t[1] + profile->t[2] + profile->t[3] + 2.0 * profile->t[4] + af / j_max);
            profile->t[6] = profile->t[4] + af / j_max;

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0_ACC1_VEL, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    if ((-a0 + 4.0 * a_max - af) / j_max < tf) {
        const double denominator = 12.0 * a_max * j_max * (a0_a0 + af_af - 2.0 * (a0 + af) * a_max + 2.0 * (a_max * a_max - a_max * j_max * tf + j_max * vd));
        if (fabs(denominator) > DBL_EPSILON) {
            clear_times(profile);
            profile->t[0] = (-a0 + a_max) / j_max;
            profile->t[1] = (3.0 * (a0_p4 + af_p4) - 4.0 * (a0_p3 + af_p3) * a_max - 4.0 * af_p3 * a_max
                    + 24.0 * (a0 + af) * a_max * a_max * a_max
                    - 6.0 * (af_af + a0_a0) * (a_max * a_max - 2.0 * j_max * vd)
                    + 6.0 * a0_a0 * (af_af - 2.0 * af * a_max - 2.0 * a_max * j_max * tf)
                    - 12.0 * a_max * a_max * (2.0 * a_max * a_max - 2.0 * a_max * j_max * tf + j_max * vd)
                    - 24.0 * af * a_max * j_max * vd
                    + 12.0 * j_max_j_max * (2.0 * a_max * g1 + vd_vd)) / denominator;
            profile->t[2] = a_max / j_max;
            profile->t[3] = (-a0_a0 - af_af + 2.0 * a_max * (a0 + af - 2.0 * a_max) - 2.0 * j_max * vd) / (2.0 * a_max * j_max) + tf;
            profile->t[4] = profile->t[2];
            profile->t[5] = tf - (profile->t[0] + profile->t[1] + profile->t[2] + profile->t[3] + 2.0 * profile->t[4] - af / j_max);
            profile->t[6] = profile->t[4] - af / j_max;

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDUD, RUCKIG_PROFILE_LIMITS_ACC0_ACC1_VEL, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    return false;
}

static bool time_acc0_vel(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double vd = vf - v0;
    const double ad = af - a0;
    const double vd_vd = vd * vd;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;
    const double ph1 = 12.0 * j_max * (-a_max * a_max * vd - j_max * vd_vd + 2.0 * a_max * j_max * (-pd + tf * vf));
    const double t_min = -af / j_max;
    const double t_max = fmin(tf - (2.0 * a_max - a0) / j_max, -a_min / j_max);
    ruckig_root_set4_t roots;
    size_t i;

    if (tf < fmax((-a0 + a_max) / j_max, 0.0) + fmax(a_max / j_max, 0.0)) {
        return false;
    }

    roots = ruckig_solve_quart_monic(
        (2.0 * a_max) / j_max,
        (a0_a0 - af_af + 2.0 * ad * a_max + a_max * a_max + 2.0 * j_max * (vd - a_max * tf)) / j_max_j_max,
        0.0,
        -(-3.0 * (a0_p4 + af_p4) + 4.0 * (af_p3 + 2.0 * a0_p3) * a_max
            - 12.0 * a0 * a_max * (af_af - 2.0 * j_max * vd)
            + 6.0 * a0_a0 * (af_af - a_max * a_max - 2.0 * j_max * vd)
            + 6.0 * af_af * (a_max * a_max - 2.0 * a_max * j_max * tf + 2.0 * j_max * vd)
            + ph1) / (12.0 * j_max_j_max * j_max_j_max)
    );

    for (i = 0; i < roots.count; ++i) {
        double t = roots.values[i];
        double h1;

        if (t < t_min || t > t_max) {
            continue;
        }

        if (t > DBL_EPSILON) {
            h1 = j_max * t * t + vd;
            {
                const double orig = (-3.0 * (a0_p4 + af_p4) + 4.0 * (af_p3 + 2.0 * a0_p3) * a_max
                        - 24.0 * af * a_max * j_max_j_max * t * t
                        - 12.0 * a0 * a_max * (af_af - 2.0 * j_max * h1)
                        + 6.0 * a0_a0 * (af_af - a_max * a_max - 2.0 * j_max * h1)
                        + 6.0 * af_af * (a_max * a_max - 2.0 * a_max * j_max * tf + 2.0 * j_max * h1)
                        - 12.0 * j_max * (a_max * a_max * h1 + j_max * h1 * h1 + 2.0 * a_max * j_max * (pd + j_max * t * t * (t - tf) - tf * vf)))
                    / (24.0 * a_max * j_max_j_max);
                const double deriv = -t * (a0_a0 - af_af + 2.0 * a_max * (ad - j_max * tf) + a_max * a_max + 3.0 * a_max * j_max * t + 2.0 * j_max * h1) / a_max;
                if (fabs(deriv) > DBL_EPSILON) {
                    t -= orig / deriv;
                }
            }
        }

        h1 = ((a0_a0 - af_af) / 2.0 + j_max * (j_max * t * t + vd)) / a_max;

        clear_times(profile);
        profile->t[0] = (-a0 + a_max) / j_max;
        profile->t[1] = (h1 - a_max) / j_max;
        profile->t[2] = a_max / j_max;
        profile->t[3] = tf - (h1 + ad + a_max) / j_max - 2.0 * t;
        profile->t[4] = t;
        profile->t[6] = af / j_max + t;

        if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0_VEL, tf, j_max, v_max, v_min, a_max, a_min)) {
            return true;
        }
    }

    roots = ruckig_solve_quart_monic(
        (-2.0 * a_max) / j_max,
        -(a0_a0 + af_af - 2.0 * (a0 + af) * a_max + a_max * a_max + 2.0 * j_max * (vd - a_max * tf)) / j_max_j_max,
        0.0,
        (3.0 * (a0_p4 + af_p4) - 4.0 * (af_p3 + 2.0 * a0_p3) * a_max
            + 6.0 * a0_a0 * (af_af + a_max * a_max + 2.0 * j_max * vd)
            - 12.0 * a0 * a_max * (af_af + 2.0 * j_max * vd)
            + 6.0 * af_af * (a_max * a_max - 2.0 * a_max * j_max * tf + 2.0 * j_max * vd)
            - ph1) / (12.0 * j_max_j_max * j_max_j_max)
    );

    for (i = 0; i < roots.count; ++i) {
        double t = roots.values[i];
        double h1;
        const double t_min = af / j_max;
        const double t_max = fmin(tf - a_max / j_max, a_max / j_max);

        if (t < t_min || t > t_max) {
            continue;
        }

        h1 = j_max * t * t - vd;
        {
            const double orig = -(3.0 * (a0_p4 + af_p4) - 4.0 * (2.0 * a0_p3 + af_p3) * a_max
                    + 24.0 * af * a_max * j_max_j_max * t * t
                    - 12.0 * a0 * a_max * (af_af - 2.0 * j_max * h1)
                    + 6.0 * a0_a0 * (af_af + a_max * a_max - 2.0 * j_max * h1)
                    + 6.0 * af_af * (a_max * a_max - 2.0 * j_max * (tf * a_max + h1))
                    + 12.0 * j_max * (-a_max * a_max * h1 + j_max * h1 * h1 - 2.0 * a_max * j_max * (-pd + j_max * t * t * (t - tf) + tf * vf)))
                / (24.0 * a_max * j_max_j_max);
            const double deriv = t * (a0_a0 + af_af - 2.0 * j_max * h1 - 2.0 * (a0 + af + j_max * tf) * a_max + a_max * a_max + 3.0 * a_max * j_max * t) / a_max;
            if (fabs(deriv) > DBL_EPSILON) {
                t -= orig / deriv;
            }
        }

        h1 = ((a0_a0 + af_af) / 2.0 + j_max * (vd - j_max * t * t)) / a_max;

        clear_times(profile);
        profile->t[0] = (-a0 + a_max) / j_max;
        profile->t[1] = (h1 - a_max) / j_max;
        profile->t[2] = a_max / j_max;
        profile->t[3] = tf - (h1 - a0 - af + a_max) / j_max - 2.0 * t;
        profile->t[4] = t;
        profile->t[6] = -(af / j_max) + t;

        if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDUD, RUCKIG_PROFILE_LIMITS_ACC0_VEL, tf, j_max, v_max, v_min, a_max, a_min)) {
            return true;
        }
    }

    return false;
}

static bool time_acc1_vel(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double vd = vf - v0;
    const double ad = af - a0;
    const double vd_vd = vd * vd;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;
    const double g1 = -pd + tf * v0;
    ruckig_root_set4_t roots;
    size_t i;

    if (fabs(j_max) < DBL_EPSILON || fabs(a_min) < DBL_EPSILON || fabs(a_max) < DBL_EPSILON) {
        return false;
    }

    {
        const double ph1 = a0_a0 + af_af - a_min * (a0 + 2.0 * af - a_min) - 2.0 * j_max * (vd - a_min * tf);
        const double ph2 = 2.0 * a_min * (j_max * g1 + af * vd) - a_min * a_min * vd + j_max * vd_vd;
        const double ph3 = af_af + a_min * (a_min - 2.0 * af) - 2.0 * j_max * (vd - a_min * tf);
        const double t_min = -a0 / j_max;
        const double t_max = fmin((tf + 2.0 * a_min / j_max - (a0 + af) / j_max) / 2.0, (a_max - a0) / j_max);

        roots = ruckig_solve_quart_monic(
            (2.0 * (2.0 * a0 - a_min)) / j_max,
            (4.0 * a0_a0 + ph1 - 3.0 * a0 * a_min) / j_max_j_max,
            (2.0 * a0 * ph1) / (j_max_j_max * j_max),
            (3.0 * (a0_p4 + af_p4) - 4.0 * (a0_p3 + 2.0 * af_p3) * a_min
                + 6.0 * af_af * (a_min * a_min - 2.0 * j_max * vd)
                + 12.0 * j_max * ph2 + 6.0 * a0_a0 * ph3) / (12.0 * j_max_j_max * j_max_j_max)
        );

        for (i = 0; i < roots.count; ++i) {
            double t = roots.values[i];
            double h1;

            if (t < t_min || t > t_max) {
                continue;
            }

            if (fabs(a0 + j_max * t) > 16.0 * DBL_EPSILON) {
                const double h0 = j_max * t * t;
                const double orig = -pd + (3.0 * (a0_p4 + af_p4) - 8.0 * af_p3 * a_min - 4.0 * a0_p3 * a_min
                        + 6.0 * af_af * (a_min * a_min + 2.0 * j_max * (h0 - vd))
                        + 6.0 * a0_a0 * (af_af - 2.0 * af * a_min + a_min * a_min + 2.0 * a_min * j_max * (-2.0 * t + tf) + 2.0 * j_max * (5.0 * h0 - vd))
                        + 24.0 * a0 * j_max * t * (a0_a0 + af_af - 2.0 * af * a_min + a_min * a_min + 2.0 * j_max * (a_min * (-t + tf) + h0 - vd))
                        - 24.0 * af * a_min * j_max * (h0 - vd)
                        + 12.0 * j_max * (a_min * a_min * (h0 - vd) + j_max * (h0 - vd) * (h0 - vd)))
                    / (24.0 * a_min * j_max_j_max) + h0 * (tf - t) + tf * v0;
                const double deriv = (a0 + j_max * t) * ((a0_a0 + af_af) / (a_min * j_max) + (a_min - a0 - 2.0 * af) / j_max + (4.0 * a0 * t + 2.0 * h0 - 2.0 * vd) / a_min + 2.0 * tf - 3.0 * t);
                if (fabs(deriv) > DBL_EPSILON) {
                    t -= orig / deriv;
                }
            }

            h1 = -((a0_a0 + af_af) / 2.0 + j_max * (-vd + 2.0 * a0 * t + j_max * t * t)) / a_min;

            clear_times(profile);
            profile->t[0] = t;
            profile->t[2] = a0 / j_max + t;
            profile->t[3] = tf - (h1 - a_min + a0 + af) / j_max - 2.0 * t;
            profile->t[4] = -a_min / j_max;
            profile->t[5] = (h1 + a_min) / j_max;
            profile->t[6] = profile->t[4] + af / j_max;

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC1_VEL, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    {
        const double ph1 = a0_a0 - af_af + (2.0 * af - a0) * a_max - a_max * a_max - 2.0 * j_max * (vd - a_max * tf);
        const double ph2 = a_max * a_max + 2.0 * j_max * vd;
        const double ph3 = af_af + ph2 - 2.0 * a_max * (af + j_max * tf);
        const double ph4 = 2.0 * a_max * j_max * g1 + a_max * a_max * vd + j_max * vd_vd;
        const double t_min = -a0 / j_max;
        const double t_max = fmin((tf + ad / j_max - 2.0 * a_max / j_max) / 2.0, (a_max - a0) / j_max);

        roots = ruckig_solve_quart_monic(
            (4.0 * a0 - 2.0 * a_max) / j_max,
            (4.0 * a0_a0 - 3.0 * a0 * a_max + ph1) / j_max_j_max,
            (2.0 * a0 * ph1) / (j_max_j_max * j_max),
            (3.0 * (a0_p4 + af_p4) - 4.0 * (a0_p3 + 2.0 * af_p3) * a_max
                - 24.0 * af * a_max * j_max * vd + 12.0 * j_max * ph4
                - 6.0 * a0_a0 * ph3 + 6.0 * af_af * ph2) / (12.0 * j_max_j_max * j_max_j_max)
        );

        for (i = 0; i < roots.count; ++i) {
            const double t = roots.values[i];
            double h1;

            if (t > t_max || t < t_min) {
                continue;
            }

            h1 = ((a0_a0 - af_af) / 2.0 + j_max_j_max * t * t - j_max * (vd - 2.0 * a0 * t)) / a_max;

            clear_times(profile);
            profile->t[0] = t;
            profile->t[2] = t + a0 / j_max;
            profile->t[3] = tf + (h1 + ad - a_max) / j_max - 2.0 * t;
            profile->t[4] = a_max / j_max;
            profile->t[5] = -(h1 + a_max) / j_max;
            profile->t[6] = profile->t[4] - af / j_max;

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDUD, RUCKIG_PROFILE_LIMITS_ACC1_VEL, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    return false;
}

static bool time_acc1(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double tf_tf = tf * tf;
    const double vd = vf - v0;
    const double vd_vd = vd * vd;
    const double ad = af - a0;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;
    const double g1 = -pd + tf * v0;
    const double g2 = -2.0 * pd + tf * (v0 + vf);

    if (fabs(j_max) < DBL_EPSILON) {
        return false;
    }

    {
        const double radicand0 = j_max_j_max * (a0_p4 + af_p4 - 4.0 * af_p3 * j_max * tf
            + 6.0 * af_af * j_max_j_max * tf_tf
            - 4.0 * a0_p3 * (af - j_max * tf)
            + 6.0 * a0_a0 * (af - j_max * tf) * (af - j_max * tf)
            + 24.0 * af * j_max_j_max * g1
            - 4.0 * a0 * (af_p3 - 3.0 * af_af * j_max * tf + 6.0 * j_max_j_max * (-pd + tf * vf))
            - 12.0 * j_max_j_max * (-vd_vd + j_max * tf * g2)) / 3.0;
        if (radicand0 >= 0.0) {
            const double h0 = sqrt(radicand0) / j_max;
            const double radicand1 = (a0_a0 + af_af - 2.0 * a0 * af - 2.0 * ad * j_max * tf + 2.0 * h0) / j_max_j_max + tf_tf;
            if (radicand1 >= 0.0 && fabs(-ad + j_max * tf) > DBL_EPSILON) {
                const double h1 = sqrt(radicand1);
                clear_times(profile);
                profile->t[0] = -(a0_a0 + af_af + 2.0 * a0 * (j_max * tf - af) - 2.0 * j_max * vd + h0) / (2.0 * j_max * (-ad + j_max * tf));
                profile->t[2] = (tf - h1) / 2.0 - ad / (2.0 * j_max);
                profile->t[5] = h1;
                profile->t[6] = tf - (profile->t[0] + profile->t[2] + profile->t[5]);

                if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC1, tf, j_max, v_max, v_min, a_max, a_min)) {
                    return true;
                }
            }
        }
    }

    {
        const double radicand0 = j_max_j_max * (a0_p4 + af_p4 + 4.0 * (af_p3 - a0_p3) * j_max * tf
            + 6.0 * af_af * j_max_j_max * tf_tf
            + 6.0 * a0_a0 * (af + j_max * tf) * (af + j_max * tf)
            + 24.0 * af * j_max_j_max * g1
            - 4.0 * a0 * (a0_a0 * af + af_p3 + 3.0 * af_af * j_max * tf + 6.0 * j_max_j_max * (-pd + tf * vf))
            + 12.0 * j_max_j_max * (vd_vd + j_max * tf * g2)) / 3.0;
        if (radicand0 >= 0.0) {
            const double h0 = sqrt(radicand0) / j_max;
            const double radicand1 = (a0_a0 + af_af - 2.0 * a0 * af + 2.0 * ad * j_max * tf + 2.0 * h0) / j_max_j_max + tf_tf;
            if (radicand1 >= 0.0 && fabs(ad + j_max * tf) > DBL_EPSILON) {
                const double h1 = sqrt(radicand1);
                clear_times(profile);
                profile->t[2] = -(a0_a0 + af_af - 2.0 * a0 * af + 2.0 * j_max * (vd - a0 * tf) + h0) / (2.0 * j_max * (ad + j_max * tf));
                profile->t[4] = ad / (2.0 * j_max) + (tf - h1) / 2.0;
                profile->t[5] = h1;
                profile->t[6] = tf - (profile->t[5] + profile->t[4] + profile->t[2]);

                if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDUD, RUCKIG_PROFILE_LIMITS_ACC1, tf, j_max, v_max, v_min, a_max, a_min)) {
                    return true;
                }
            }
        }
    }

    return false;
}

static bool time_acc0(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double tf_tf = tf * tf;
    const double vd = vf - v0;
    const double ad = af - a0;
    const double ad_ad = ad * ad;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double af_p3 = af_af * af;
    const double j_max_j_max = j_max * j_max;
    const double g1 = -pd + tf * v0;
    const double g2 = -2.0 * pd + tf * (v0 + vf);

    if (fabs(j_max) < DBL_EPSILON) {
        return false;
    }

    {
        const double radicand = ad_ad / (2.0 * j_max_j_max)
            - ad * (a_max - a0) / j_max_j_max
            + (a_max * tf - vd) / j_max;

        if (radicand >= 0.0) {
            const double h1 = sqrt(radicand);

            clear_times(profile);
            profile->t[0] = (a_max - a0) / j_max;
            profile->t[1] = tf - ad / j_max - 2.0 * h1;
            profile->t[2] = h1;
            profile->t[4] = (af - a_max) / j_max + h1;

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDUD, RUCKIG_PROFILE_LIMITS_NONE, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    {
        const double h0a = -a0_a0 + af_af - 2.0 * ad * a_max + 2.0 * j_max * (a_max * tf - vd);
        const double h0b = a0_p3 + 2.0 * af_p3 - 6.0 * af_af * a_max
            - 3.0 * a0_a0 * (af - j_max * tf)
            - 3.0 * a0 * a_max * (a_max - 2.0 * af + 2.0 * j_max * tf)
            - 3.0 * j_max * (j_max * (-2.0 * pd + a_max * tf_tf + 2.0 * tf * v0)
                + a_max * (a_max * tf - 2.0 * vd))
            + 3.0 * af * (a_max * a_max + 2.0 * a_max * j_max * tf - 2.0 * j_max * vd);
        const double radicand = 4.0 * h0b * h0b - 18.0 * h0a * h0a * h0a;
        const double h1 = 3.0 * j_max * h0a;

        if (radicand >= 0.0 && fabs(h1) > DBL_EPSILON) {
            const double h0 = fabs(j_max) * sqrt(radicand);

            clear_times(profile);
            profile->t[0] = (-a0 + a_max) / j_max;
            profile->t[1] = (-a0_p3 + af_p3 + af_af * (-6.0 * a_max + 3.0 * j_max * tf)
                    + a0_a0 * (-3.0 * af + 6.0 * a_max + 3.0 * j_max * tf)
                    + 6.0 * af * (a_max * a_max - j_max * vd)
                    + 3.0 * a0 * (af_af - 2.0 * (a_max * a_max + j_max * vd))
                    - 6.0 * j_max * (a_max * (a_max * tf - 2.0 * vd) + j_max * g2))
                / h1;
            profile->t[2] = -(ad + h0 / h1) / (2.0 * j_max) + tf / 2.0 - profile->t[1] / 2.0;
            profile->t[3] = h0 / (j_max * h1);
            profile->t[6] = tf - (profile->t[0] + profile->t[1] + profile->t[2] + profile->t[3]);

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    {
        const double h0a = a0_p3 + 2.0 * af_p3 - 6.0 * (af_af + a_max * a_max) * a_max
            - 6.0 * (a0 + af) * a_max * j_max * tf
            + 9.0 * a_max * a_max * (af + j_max * tf)
            + 3.0 * a0 * a_max * (-2.0 * af + 3.0 * a_max)
            + 3.0 * a0_a0 * (af - 2.0 * a_max + j_max * tf)
            - 6.0 * j_max_j_max * g1
            + 6.0 * (af - a_max) * j_max * vd
            - 3.0 * a_max * j_max_j_max * tf_tf;
        const double h0b = a0_a0 + af_af
            + 2.0 * (a_max * a_max - (a0 + af) * a_max + j_max * (vd - a_max * tf));
        const double radicand = 4.0 * h0a * h0a - 18.0 * h0b * h0b * h0b;
        const double h2 = 6.0 * j_max * h0b;

        if (radicand >= 0.0 && fabs(h2) > DBL_EPSILON) {
            const double h1 = fabs(j_max) / j_max * sqrt(radicand);

            clear_times(profile);
            profile->t[0] = (-a0 + a_max) / j_max;
            profile->t[1] = ad / j_max - 2.0 * profile->t[0] - (2.0 * h0a - h1) / h2 + tf;
            profile->t[2] = -(2.0 * h0a + h1) / h2;
            profile->t[3] = (2.0 * h0a - h1) / h2;
            profile->t[4] = tf - (profile->t[0] + profile->t[1] + profile->t[2] + profile->t[3]);

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    return false;
}

static bool time_acc0_acc1(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double tf_tf = tf * tf;
    const double vd = vf - v0;
    const double g1 = -pd + tf * v0;
    const double g2 = -2.0 * pd + tf * (v0 + vf);
    const double ad = af - a0;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double af_p3 = af_af * af;
    double jf;

    if (fabs(a0) < DBL_EPSILON && fabs(af) < DBL_EPSILON) {
        const double h1 = 2.0 * a_min * g1 + vd * vd + a_max * (2.0 * pd + a_min * tf_tf - 2.0 * tf * vf);
        const double h2 = (a_max - a_min) * (-a_min * vd + a_max * (a_min * tf - vd));
        if (fabs(h1) > DBL_EPSILON && fabs(h2) > DBL_EPSILON) {
            jf = h2 / h1;
            if (fabs(jf) > DBL_EPSILON) {
                clear_times(profile);
                profile->t[0] = a_max / jf;
                profile->t[1] = (-2.0 * a_max * h1 + a_min * a_min * g2) / h2;
                profile->t[2] = profile->t[0];
                profile->t[4] = -a_min / jf;
                profile->t[5] = tf - (2.0 * profile->t[0] + profile->t[1] + 2.0 * profile->t[4]);
                profile->t[6] = profile->t[4];

                return ruckig_profile_check_with_timing_guarded(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0_ACC1, tf, jf, v_max, v_min, a_max, a_min, j_max);
            }
        }
    }

    {
        const double h_den = 2.0 * a_min * g1 + vd * vd + a_max * (2.0 * pd + a_min * tf_tf - 2.0 * tf * vf);
        const double h_a = (a_max - a_min) * (-a_min * vd + a_max * (a_min * tf - vd))
            - af_af * (a_max * tf - vd)
            + 2.0 * af * a_min * (a_max * tf - vd)
            + a0_a0 * (a_min * tf + v0 - vf)
            - 2.0 * a0 * a_max * (a_min * tf - vd);
        const double h_b = 3.0 * a0_p3 - 3.0 * af_p3 + 12.0 * a_max * a_min * (-a_max + a_min)
            + 4.0 * af_af * (a_max + 2.0 * a_min)
            + a0 * (-3.0 * af_af + 8.0 * af * (a_min - a_max) + 6.0 * (a_max * a_max + 2.0 * a_max * a_min - a_min * a_min))
            + 6.0 * af * (a_max * a_max - 2.0 * a_max * a_min - a_min * a_min)
            + a0_a0 * (3.0 * af - 4.0 * (2.0 * a_max + a_min));
        const double radicand = 144.0 * h_a * h_a + 48.0 * ad * h_b * h_den;

        if (radicand >= 0.0 && fabs(h_den) > DBL_EPSILON) {
            const double h1 = sqrt(radicand);
            jf = -(3.0 * af_af * a_max * tf - 3.0 * a0_a0 * a_min * tf
                    - 6.0 * ad * a_max * a_min * tf
                    + 3.0 * a_max * a_min * (a_min - a_max) * tf
                    + 3.0 * (a0_a0 - af_af) * vd
                    + 6.0 * vd * (af * a_min - a0 * a_max)
                    + 3.0 * (a_max * a_max - a_min * a_min) * vd
                    + h1 / 4.0) / (6.0 * h_den);

            if (fabs(jf) > DBL_EPSILON && fabs(a_max - a_min) > DBL_EPSILON) {
                clear_times(profile);
                profile->t[0] = (a_max - a0) / jf;
                profile->t[1] = (a0_a0 - af_af + 2.0 * ad * a_min
                        - 2.0 * (a_max * a_max - 2.0 * a_max * a_min + a_min * a_min + a_min * jf * tf - jf * vd))
                    / (2.0 * (a_max - a_min) * jf);
                profile->t[2] = a_max / jf;
                profile->t[4] = -a_min / jf;
                profile->t[5] = tf - (profile->t[0] + profile->t[1] + profile->t[2] + 2.0 * profile->t[4] + af / jf);
                profile->t[6] = profile->t[4] + af / jf;

                if (ruckig_profile_check_with_timing_guarded(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0_ACC1, tf, jf, v_max, v_min, a_max, a_min, j_max)) {
                    return true;
                }
            }
        }
    }

    return false;
}

static bool time_none_udud_t0246(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double tf_tf = tf * tf;
    const double tf_p3 = tf_tf * tf;
    const double tf_p4 = tf_tf * tf_tf;
    const double vd = vf - v0;
    const double vd_vd = vd * vd;
    const double ad = af - a0;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;
    const double g2 = -2.0 * pd + tf * (v0 + vf);
    double h0a;
    double h0b;
    double h0c;
    double h0_radicand;
    double h0;
    double h1;
    double h2;
    double h3;

    if (fabs(j_max) < DBL_EPSILON) {
        return false;
    }

    h0a = a0_p3 - af_p3 - 3.0 * af_af * j_max * tf
        + 9.0 * af * j_max_j_max * tf_tf
        - 3.0 * a0_a0 * (af + j_max * tf)
        + 3.0 * a0 * (af + j_max * tf) * (af + j_max * tf)
        + 3.0 * j_max_j_max * (8.0 * pd + j_max * tf_p3 - 8.0 * tf * vf);
    h0b = a0_a0 + af_af - 2.0 * af * j_max * tf - 2.0 * a0 * (af + j_max * tf)
        - j_max * (j_max * tf_tf + 4.0 * v0 - 4.0 * vf);
    h0c = a0_p4 + af_p4 + 4.0 * af_p3 * j_max * tf + 6.0 * af_af * j_max_j_max * tf_tf
        - 3.0 * j_max_j_max * j_max_j_max * tf_p4
        - 4.0 * a0_p3 * (af + j_max * tf)
        + 6.0 * a0_a0 * (af + j_max * tf) * (af + j_max * tf)
        - 12.0 * af * j_max_j_max * (8.0 * pd + j_max * tf_p3 - 8.0 * tf * v0)
        + 48.0 * j_max_j_max * vd_vd + 48.0 * j_max_j_max * j_max * tf * g2
        - 4.0 * a0 * (af_p3 + 3.0 * af_af * j_max * tf - 9.0 * af * j_max_j_max * tf_tf
            - 3.0 * j_max_j_max * (8.0 * pd + j_max * tf_p3 - 8.0 * tf * vf));
    h0_radicand = 2.0 * j_max_j_max * (2.0 * h0a * h0a - 3.0 * h0b * h0c);
    if (h0_radicand < 0.0) {
        return false;
    }

    h0 = sqrt(h0_radicand) / j_max;
    h1 = 12.0 * j_max * (-a0_a0 - af_af + 2.0 * af * j_max * tf
        + 2.0 * a0 * (af + j_max * tf)
        + j_max * (j_max * tf_tf + 4.0 * v0 - 4.0 * vf));
    if (fabs(h1) < DBL_EPSILON) {
        return false;
    }

    h2 = -4.0 * a0_p3 + 4.0 * af_p3 + 12.0 * a0_a0 * af - 12.0 * a0 * af_af
        + 48.0 * j_max_j_max * pd + 12.0 * (a0_a0 - af_af) * j_max * tf
        - 24.0 * j_max_j_max * tf * (v0 + vf) + 24.0 * ad * j_max * vd;
    h3 = 2.0 * a0_p3 - 2.0 * af_p3 - 6.0 * a0_a0 * af + 6.0 * a0 * af_af;

    clear_times(profile);
    profile->t[0] = (h3 - 48.0 * j_max_j_max * (tf * vf - pd)
            - 6.0 * (a0_a0 + af_af) * j_max * tf + 12.0 * a0 * af * j_max * tf
            + 6.0 * (a0 + 3.0 * af + j_max * tf) * tf_tf * j_max_j_max - h0) / h1;
    profile->t[2] = (h2 + h0) / h1;
    profile->t[4] = (-h2 + h0) / h1;
    profile->t[6] = (-h3 + 48.0 * j_max_j_max * (tf * v0 - pd)
            - 6.0 * (a0_a0 + af_af) * j_max * tf + 12.0 * a0 * af * j_max * tf
            + 6.0 * (af + 3.0 * a0 + j_max * tf) * tf_tf * j_max_j_max - h0) / h1;

    return ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDUD, RUCKIG_PROFILE_LIMITS_NONE, tf, j_max, v_max, v_min, a_max, a_min);
}

/* Enumerates no-limit synchronization families in one place to keep candidate order auditable. */
static bool time_none(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double tf_tf = tf * tf;
    const double tf_p3 = tf_tf * tf;
    const double tf_p4 = tf_tf * tf_tf;
    const double vd = vf - v0;
    const double vd_vd = vd * vd;
    const double ad = af - a0;
    const double ad_ad = ad * ad;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double af_p3 = af_af * af;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p4 = af_af * af_af;
    const double a0_p5 = a0_p4 * a0;
    const double af_p5 = af_p4 * af;
    const double a0_p6 = a0_p4 * a0_a0;
    const double af_p6 = af_p4 * af_af;
    const double j_max_j_max = j_max * j_max;
    const double g1 = -pd + tf * v0;
    const double g2 = -2.0 * pd + tf * (v0 + vf);

    if (fabs(j_max) < DBL_EPSILON) {
        return false;
    }

    if (fabs(v0) < DBL_EPSILON && fabs(a0) < DBL_EPSILON && fabs(af) < DBL_EPSILON) {
        const double h1 = sqrt(tf_tf * vf * vf + ruckig_pow2(4.0 * pd - tf * vf));
        const double jf = 4.0 * (4.0 * pd - 2.0 * tf * vf + h1) / tf_p3;

        clear_times(profile);
        profile->t[0] = tf / 4.0;
        profile->t[2] = 2.0 * profile->t[0];
        profile->t[6] = profile->t[0];

        if (ruckig_profile_check_with_timing_guarded(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, jf, v_max, v_min, a_max, a_min, j_max)) {
            return true;
        }
    }

    if (fabs(a0) < DBL_EPSILON && fabs(af) < DBL_EPSILON) {
        double polynom[5];
        ruckig_root_set4_t roots;
        size_t i;

        polynom[0] = 1.0;
        polynom[1] = -2.0 * tf;
        polynom[2] = 2.0 * vd / j_max + tf_tf;
        polynom[3] = 4.0 * (pd - tf * vf) / j_max;
        polynom[4] = (vd_vd + j_max * tf * g2) / j_max_j_max;

        roots = ruckig_solve_quart_monic(polynom[1], polynom[2], polynom[3], polynom[4]);
        for (i = 0; i < roots.count; ++i) {
            double t = roots.values[i];
            if (t > tf / 2.0 || t > (a_max - a0) / j_max) {
                continue;
            }

            {
                const double h1 = (j_max * t * (t - tf) + vd) / (j_max * (2.0 * t - tf));
                const double h2 = (2.0 * j_max * t * (t - tf) + j_max * tf_tf - 2.0 * vd) / (j_max * (2.0 * t - tf) * (2.0 * t - tf));
                const double orig = (-2.0 * pd + 2.0 * tf * v0 + h1 * h1 * j_max * (tf - 2.0 * t) + j_max * tf * (2.0 * h1 * t - t * t - (h1 - t) * tf)) / 2.0;
                const double deriv = (j_max * tf * (2.0 * t - tf) * (h2 - 1.0)) / 2.0 + h1 * j_max * (tf - (2.0 * t - tf) * h2 - h1);
                if (fabs(deriv) > DBL_EPSILON) {
                    t -= orig / deriv;
                }
            }

            clear_times(profile);
            profile->t[0] = t;
            profile->t[2] = (j_max * t * (t - tf) + vd) / (j_max * (2.0 * t - tf));
            profile->t[3] = tf - 2.0 * t;
            profile->t[4] = t - profile->t[2];

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    if (time_none_udud_t0246(profile, tf, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max)) {
        return true;
    }

    {
        double polynom[5];
        ruckig_root_set4_t roots;
        size_t i;
        const double ph1 = af + j_max * tf;
        const double t_min = ad / j_max;
        const double t_max = fmin((a_max - a0) / j_max, (ad / j_max + tf) / 2.0);

        if (fabs(j_max) > DBL_EPSILON) {
            polynom[0] = 1.0;
            polynom[1] = -2.0 * (ad + j_max * tf) / j_max;
            polynom[2] = 2.0 * (a0_a0 + af_af + j_max * (af * tf + vd) - 2.0 * a0 * ph1) / j_max_j_max + tf_tf;
            polynom[3] = 2.0 * (a0_p3 - af_p3 - 3.0 * af_af * j_max * tf + 3.0 * a0 * ph1 * (ph1 - a0) - 6.0 * j_max_j_max * (-pd + tf * vf)) / (3.0 * j_max_j_max * j_max);
            polynom[4] = (a0_p4 + af_p4 + 4.0 * af_p3 * j_max * tf - 4.0 * a0_p3 * ph1 + 6.0 * a0_a0 * ph1 * ph1 + 24.0 * j_max_j_max * af * g1 - 4.0 * a0 * (af_p3 + 3.0 * af_af * j_max * tf + 6.0 * j_max_j_max * (-pd + tf * vf)) + 6.0 * j_max_j_max * af_af * tf_tf + 12.0 * j_max_j_max * (vd_vd + j_max * tf * g2)) / (12.0 * j_max_j_max * j_max_j_max);

            roots = ruckig_solve_quart_monic(polynom[1], polynom[2], polynom[3], polynom[4]);
            for (i = 0; i < roots.count; ++i) {
                double t = roots.values[i];
                if (t < t_min || t > t_max) {
                    continue;
                }

                {
                    const double h0 = j_max * (2.0 * t - tf) - ad;
                    const double h1 = (ad_ad - 2.0 * af * j_max * t + 2.0 * a0 * j_max * (t - tf) + 2.0 * j_max * (j_max * t * (t - tf) + vd)) / (2.0 * j_max * h0);
                    const double h2 = (-ad_ad + 2.0 * j_max_j_max * (tf_tf + t * (t - tf)) + (a0 + af) * j_max * tf - ad * h0 - 2.0 * j_max * vd) / (h0 * h0);
                    const double orig = (-a0_p3 + af_p3 + 3.0 * ad_ad * j_max * (h1 - t) + 3.0 * ad * j_max_j_max * (h1 - t) * (h1 - t) - 3.0 * a0 * af * ad + 3.0 * j_max_j_max * (a0 * tf_tf - 2.0 * pd + 2.0 * tf * v0 + h1 * h1 * j_max * (tf - 2.0 * t) + j_max * tf * (2.0 * h1 * t - t * t - (h1 - t) * tf))) / (6.0 * j_max_j_max);
                    const double deriv = (h0 * (-ad + j_max * tf) * (h2 - 1.0)) / (2.0 * j_max) + h1 * (-ad + j_max * (tf - h1) - h0 * h2);
                    if (fabs(deriv) > DBL_EPSILON) {
                        t -= orig / deriv;
                    }
                }

                clear_times(profile);
                profile->t[0] = t;
                profile->t[2] = (ad_ad + 2.0 * j_max * (-a0 * tf - ad * t + j_max * t * (t - tf) + vd)) / (2.0 * j_max * (-ad + j_max * (2.0 * t - tf)));
                profile->t[3] = ad / j_max + tf - 2.0 * t;
                profile->t[4] = tf - (t + profile->t[2] + profile->t[3]);

                if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, j_max, v_max, v_min, a_max, a_min)) {
                    return true;
                }
            }
        }
    }

    {
        const double h1 = 3.0 * j_max * (ad_ad + 2.0 * j_max * (a0 * tf - vd));
        const double h2 = ad_ad + 2.0 * j_max * (a0 * tf - vd);
        const double radicand = 4.0 * ruckig_pow2(2.0 * (a0_p3 - af_p3) - 6.0 * a0_a0 * (af - j_max * tf) + 6.0 * j_max_j_max * g1 + 3.0 * a0 * (2.0 * af_af - 2.0 * j_max * af * tf + j_max_j_max * tf_tf) + 6.0 * ad * j_max * vd) - 18.0 * h2 * h2 * h2;
        if (radicand >= 0.0 && fabs(h1) > DBL_EPSILON) {
            const double h0 = sqrt(radicand) / h1 * fabs(j_max) / j_max;

            clear_times(profile);
            profile->t[3] = (af_p3 - a0_p3 + 3.0 * (af_af - a0_a0) * j_max * tf - 3.0 * ad * (a0 * af + 2.0 * j_max * vd) - 6.0 * j_max_j_max * g2) / h1;
            profile->t[4] = (tf - profile->t[3] - h0) / 2.0 - ad / (2.0 * j_max);
            profile->t[5] = h0;
            profile->t[6] = (tf - profile->t[3] + ad / j_max - h0) / 2.0;

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    {
        double polynom[5];
        ruckig_root_set4_t roots;
        size_t i;
        const double ph1 = ad_ad + 2.0 * (af + a0) * j_max * tf - j_max * (j_max * tf_tf + 4.0 * vd);
        const double ph2 = j_max * tf_tf * g1 - vd * (-2.0 * pd - tf * v0 + 3.0 * tf * vf);
        const double ph3 = 5.0 * af_af - 8.0 * af * j_max * tf + 2.0 * j_max * (2.0 * j_max * tf_tf - vd);
        const double ph4 = j_max_j_max * tf_p4 - 2.0 * vd_vd + 8.0 * j_max * tf * (-pd + tf * vf);
        const double ph5 = (5.0 * af_p4 - 8.0 * af_p3 * j_max * tf - 12.0 * af_af * j_max * (j_max * tf_tf + vd) + 24.0 * af * j_max_j_max * (-2.0 * pd + j_max * tf_p3 + 2.0 * tf * vf) - 6.0 * j_max_j_max * ph4);
        const double ph6 = -vd_vd + j_max * tf * (-2.0 * pd + 3.0 * tf * v0 - tf * vf) - af * g2;
        const double t_max = (a0 - a_min) / j_max;

        if (fabs(ph1) > DBL_EPSILON) {
            polynom[0] = 1.0;
            polynom[1] = -(4.0 * (a0_p3 - af_p3) - 12.0 * a0_a0 * (af - j_max * tf) + 6.0 * a0 * (2.0 * af_af - 2.0 * af * j_max * tf + j_max * (j_max * tf_tf - 2.0 * vd)) + 6.0 * af * j_max * (3.0 * j_max * tf_tf + 2.0 * vd) - 6.0 * j_max_j_max * (-4.0 * pd + j_max * tf_p3 - 2.0 * tf * v0 + 6.0 * tf * vf)) / (3.0 * j_max * ph1);
            polynom[2] = -(-a0_p4 - af_p4 + 4.0 * a0_p3 * (af - j_max * tf) + a0_a0 * (-6.0 * af_af + 8.0 * af * j_max * tf - 4.0 * j_max * (j_max * tf_tf - vd)) + 2.0 * af_af * j_max * (j_max * tf_tf + 2.0 * vd) - 4.0 * af * j_max_j_max * (-3.0 * pd + j_max * tf_p3 + 2.0 * tf * v0 + tf * vf) + j_max_j_max * (j_max_j_max * tf_p4 - 8.0 * vd_vd + 4.0 * j_max * tf * (-3.0 * pd + tf * v0 + 2.0 * tf * vf)) + 2.0 * a0 * (2.0 * af_p3 - 2.0 * af_af * j_max * tf + af * j_max * (-3.0 * j_max * tf_tf - 4.0 * vd) + j_max_j_max * (-6.0 * pd + j_max * tf_p3 - 4.0 * tf * v0 + 10.0 * tf * vf))) / (j_max_j_max * ph1);
            polynom[3] = -(a0_p5 - af_p5 + af_p4 * j_max * tf - 5.0 * a0_p4 * (af - j_max * tf) + 2.0 * a0_p3 * ph3 + 4.0 * af_p3 * j_max * (j_max * tf_tf + vd) + 12.0 * j_max_j_max * af * ph6 - 2.0 * a0_a0 * (5.0 * af_p3 - 9.0 * af_af * j_max * tf - 6.0 * af * j_max * vd + 6.0 * j_max_j_max * (-2.0 * pd - tf * v0 + 3.0 * tf * vf)) - 12.0 * j_max_j_max * j_max * ph2 + a0 * ph5) / (3.0 * j_max_j_max * j_max * ph1);
            polynom[4] = -(-a0_p6 - af_p6 + 6.0 * a0_p5 * (af - j_max * tf) - 48.0 * af_p3 * j_max_j_max * g1 + 72.0 * j_max_j_max * j_max * (j_max * g1 * g1 + vd_vd * vd + 2.0 * af * g1 * vd) - 3.0 * a0_p4 * ph3 - 36.0 * af_af * j_max_j_max * vd_vd + 6.0 * af_p4 * j_max * vd + 4.0 * a0_p3 * (5.0 * af_p3 - 9.0 * af_af * j_max * tf - 6.0 * af * j_max * vd + 6.0 * j_max_j_max * (-2.0 * pd - tf * v0 + 3.0 * tf * vf)) - 3.0 * a0_a0 * ph5 + 6.0 * a0 * (af_p5 - af_p4 * j_max * tf - 4.0 * af_p3 * j_max * (j_max * tf_tf + vd) + 12.0 * j_max_j_max * (-af * ph6 + j_max * ph2))) / (18.0 * j_max_j_max * j_max_j_max * ph1);

            roots = ruckig_solve_quart_monic(polynom[1], polynom[2], polynom[3], polynom[4]);
            for (i = 0; i < roots.count; ++i) {
                double t = roots.values[i];
                if (t > t_max) {
                    continue;
                }

                {
                    const double h1 = ad_ad / 2.0 + j_max * (af * t + (j_max * t - a0) * (t - tf) - vd);
                    const double h2 = -ad + j_max * (tf - 2.0 * t);
                    if (h1 < 0.0) {
                        continue;
                    }
                    const double h3 = sqrt(h1);
                    const double orig = (af_p3 - a0_p3 + 3.0 * af * j_max * t * (af + j_max * t) + 3.0 * a0_a0 * (af + j_max * t) - 3.0 * a0 * (af_af + 2.0 * af * j_max * t + j_max_j_max * (t * t - tf_tf)) + 3.0 * j_max_j_max * (-2.0 * pd + j_max * t * (t - tf) * tf + 2.0 * tf * v0)) / (6.0 * j_max_j_max) - h3 * h3 * h3 / (j_max * fabs(j_max)) + ((-ad - j_max * t) * h1) / j_max_j_max;
                    const double deriv = (6.0 * j_max * h2 * h3 / fabs(j_max) + 2.0 * (-ad - j_max * tf) * h2 - 2.0 * (3.0 * ad_ad + af * j_max * (8.0 * t - 2.0 * tf) + 4.0 * a0 * j_max * (-2.0 * t + tf) + 2.0 * j_max * (j_max * t * (3.0 * t - 2.0 * tf) - vd))) / (4.0 * j_max);
                    if (fabs(deriv) > DBL_EPSILON) {
                        t -= orig / deriv;
                    }
                }

                {
                    const double radicand = 2.0 * ad_ad + 4.0 * j_max * (ad * t + a0 * tf + j_max * t * (t - tf) - vd);
                    if (radicand < 0.0) {
                        continue;
                    }
                    const double h1 = sqrt(radicand) / fabs(j_max);

                    clear_times(profile);
                    profile->t[2] = t;
                    profile->t[3] = tf - 2.0 * t - ad / j_max - h1;
                    profile->t[4] = h1 / 2.0;
                    profile->t[6] = tf - (t + profile->t[3] + profile->t[4]);

                    if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, j_max, v_max, v_min, a_max, a_min)) {
                        return true;
                    }
                }
            }
        }
    }

    {
        const double radicand = -ad_ad + j_max * (2.0 * (a0 + af) * tf - 4.0 * vd + j_max * tf_tf);
        if (radicand >= 0.0) {
            const double h1 = sqrt(radicand) / fabs(j_max);

            clear_times(profile);
            profile->t[0] = (tf - h1 + ad / j_max) / 2.0;
            profile->t[1] = h1;
            profile->t[2] = (tf - h1 - ad / j_max) / 2.0;

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    {
        ruckig_root_set3_t roots;
        size_t i;
        roots = ruckig_solve_cubic(
            ad_ad,
            ad_ad * tf,
            (a0_a0 + af_af + 10.0 * a0 * af) * tf_tf + 24.0 * (tf * (af * v0 - a0 * vf) - pd * ad) + 12.0 * vd_vd,
            -3.0 * tf * ((a0_a0 + af_af + 2.0 * a0 * af) * tf_tf - 4.0 * vd * (a0 + af) * tf + 4.0 * vd_vd)
        );
        for (i = 0; i < roots.count; ++i) {
            const double t = roots.values[i];
            double jf;
            if (t > tf || fabs(tf - t) < DBL_EPSILON || fabs(t) < DBL_EPSILON) {
                continue;
            }
            jf = ad / (tf - t);
            if (fabs(jf) < DBL_EPSILON) {
                continue;
            }

            clear_times(profile);
            profile->t[0] = (2.0 * (vd - a0 * tf) + ad * (t - tf)) / (2.0 * jf * t);
            profile->t[1] = t;
            profile->t[6] = tf - (profile->t[0] + profile->t[1]);

            if (ruckig_profile_check_with_timing_guarded(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, jf, v_max, v_min, a_max, a_min, j_max)) {
                return true;
            }
        }
    }

    {
        if (fabs(ad - j_max * tf) > DBL_EPSILON) {
            clear_times(profile);
            profile->t[0] = (ad_ad / j_max + 2.0 * (a0 + af) * tf - j_max * tf_tf - 4.0 * vd) / (4.0 * (ad - j_max * tf));
            profile->t[2] = -ad / (2.0 * j_max) + tf / 2.0;
            profile->t[6] = tf - (profile->t[0] + profile->t[2]);

            if (ruckig_profile_check_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    return false;
}

/* Dispatches third-order position timing families; profile checks are the final acceptance gate. */
bool ruckig_position_third_step2_get_profile(
    ruckig_profile_t* profile,
    double tf,
    double p0,
    double v0,
    double a0,
    double pf,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double pd = pf - p0;
    const bool up_first = pd > tf * v0;
    const double oriented_v_max = up_first ? v_max : v_min;
    const double oriented_v_min = up_first ? v_min : v_max;
    const double oriented_a_max = up_first ? a_max : a_min;
    const double oriented_a_min = up_first ? a_min : a_max;
    const double oriented_j_limit = up_first ? j_max : -j_max;
    const double reverse_v_max = up_first ? v_min : v_max;
    const double reverse_v_min = up_first ? v_max : v_min;
    const double reverse_a_max = up_first ? a_min : a_max;
    const double reverse_a_min = up_first ? a_max : a_min;
    const double reverse_j_limit = up_first ? -j_max : j_max;
    const double tj = tf / 4.0;
    double jf;

    if (!profile || tf <= 0.0 || !isfinite(tf)) {
        return false;
    }
    if (tj <= 0.0 || fabs(oriented_j_limit) < DBL_EPSILON) {
        return false;
    }

    if (fabs(v0) < DBL_EPSILON && fabs(a0) < DBL_EPSILON && fabs(vf) < DBL_EPSILON && fabs(af) < DBL_EPSILON) {
        if (time_vel_rest_to_rest(profile, tf, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_limit)) {
            return true;
        }

        jf = pd / (2.0 * tj * tj * tj);
        if (fabs(jf) > fabs(j_max) + RUCKIG_C_JERK_LIMIT_TOLERANCE) {
            return false;
        }

        clear_times(profile);
        profile->t[0] = tj;
        profile->t[2] = tj;
        profile->t[4] = tj;
        profile->t[6] = tj;

        if (ruckig_profile_check_with_timing_guarded(
            profile,
            RUCKIG_PROFILE_SIGNS_UDDU,
            RUCKIG_PROFILE_LIMITS_NONE,
            tf,
            jf,
            oriented_v_max,
            oriented_v_min,
            oriented_a_max,
            oriented_a_min,
            oriented_j_limit
        )) {
            return true;
        }
    }

    if (time_acc0_acc1_vel(profile, tf, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_limit)) {
        return true;
    }

    if (time_vel_general_uddu(profile, tf, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_limit)) {
        return true;
    }

    if (time_vel_general_udud(profile, tf, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_limit)) {
        return true;
    }

    if (time_acc0_vel(profile, tf, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_limit)) {
        return true;
    }

    if (time_acc1_vel(profile, tf, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_limit)) {
        return true;
    }

    if (time_acc0_acc1_vel(profile, tf, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_limit)) {
        return true;
    }

    if (time_vel_general_uddu(profile, tf, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_limit)) {
        return true;
    }

    if (time_vel_general_udud(profile, tf, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_limit)) {
        return true;
    }

    if (time_acc0_vel(profile, tf, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_limit)) {
        return true;
    }

    if (time_acc1_vel(profile, tf, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_limit)) {
        return true;
    }

    if (time_acc0_acc1(profile, tf, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_limit)) {
        return true;
    }

    if (time_acc0(profile, tf, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_limit)) {
        return true;
    }

    if (time_acc1(profile, tf, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_limit)) {
        return true;
    }

    if (time_none(profile, tf, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_limit)) {
        return true;
    }

    if (time_acc0_acc1(profile, tf, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_limit)) {
        return true;
    }

    if (time_acc0(profile, tf, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_limit)) {
        return true;
    }

    if (time_acc1(profile, tf, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_limit)) {
        return true;
    }

    return time_none(profile, tf, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_limit);
}
