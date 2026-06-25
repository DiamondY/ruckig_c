#include "ruckig_c/position_first.h"

#include "ruckig_c/roots.h"

#include <float.h>
#include <math.h>
#include <string.h>

static void clear_times(ruckig_profile_t* profile) {
    memset(profile->t, 0, sizeof(profile->t));
}

static bool near_zero(double value) {
    return fabs(value) < DBL_EPSILON;
}

static bool time_all_single_step(
    ruckig_profile_t* profile,
    double pd,
    double v0,
    double a0,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    if (fabs(af - a0) > DBL_EPSILON) {
        return false;
    }

    clear_times(profile);

    if (fabs(a0) > DBL_EPSILON) {
        const double q_square = 2.0 * a0 * pd + v0 * v0;
        if (q_square < 0.0) {
            return false;
        }
        {
            const double q = sqrt(q_square);
            profile->t[3] = (-v0 + q) / a0;
            if (profile->t[3] >= 0.0
                && ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, false, 0.0, v_max, v_min, a_max, a_min)) {
                return true;
            }
            profile->t[3] = -(v0 + q) / a0;
            if (profile->t[3] >= 0.0
                && ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, false, 0.0, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    } else if (fabs(v0) > DBL_EPSILON) {
        profile->t[3] = pd / v0;
        if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, false, 0.0, v_max, v_min, a_max, a_min)) {
            return true;
        }
    } else if (fabs(pd) < DBL_EPSILON) {
        if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, false, 0.0, v_max, v_min, a_max, a_min)) {
            return true;
        }
    }

    return false;
}

static bool time_symmetric_rest_to_rest(
    ruckig_profile_t* profile,
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
    double tj;

    if (!near_zero(v0) || !near_zero(a0) || !near_zero(vf) || !near_zero(af) || near_zero(pd) || near_zero(j_max)) {
        return false;
    }

    tj = cbrt(pd / (2.0 * j_max));
    if (!(tj > 0.0) || !isfinite(tj)) {
        return false;
    }

    clear_times(profile);
    profile->t[0] = tj;
    profile->t[2] = tj;
    profile->t[4] = tj;
    profile->t[6] = tj;

    return ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, false, j_max, v_max, v_min, a_max, a_min);
}

static bool time_vel_rest_to_rest(
    ruckig_profile_t* profile,
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
    double tj;
    double t_vel;

    if (!near_zero(v0) || !near_zero(a0) || !near_zero(vf) || !near_zero(af) || near_zero(pd) || near_zero(v_max) || near_zero(j_max)) {
        return false;
    }

    tj = sqrt(v_max / j_max);
    if (!(tj > 0.0) || !isfinite(tj)) {
        return false;
    }

    t_vel = (pd - 2.0 * j_max * tj * tj * tj) / v_max;
    if (t_vel <= DBL_EPSILON || !isfinite(t_vel)) {
        return false;
    }

    clear_times(profile);
    profile->t[0] = tj;
    profile->t[2] = tj;
    profile->t[3] = t_vel;
    profile->t[4] = tj;
    profile->t[6] = tj;

    return ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_VEL, false, j_max, v_max, v_min, a_max, a_min);
}

static bool time_all_vel_uddu(
    ruckig_profile_t* profile,
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
    const double v0_v0 = v0 * v0;
    const double vf_vf = vf * vf;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;
    double t_acc0;
    double t_acc1;

    if (fabs(j_max) < DBL_EPSILON || fabs(v_max) < DBL_EPSILON) {
        return false;
    }

    clear_times(profile);
    profile->t[0] = (-a0 + a_max) / j_max;
    profile->t[1] = (a0_a0 / 2.0 - a_max * a_max - j_max * (v0 - v_max)) / (a_max * j_max);
    profile->t[2] = a_max / j_max;
    profile->t[3] = (3.0 * (a0_p4 * a_min - af_p4 * a_max)
            + 8.0 * a_max * a_min * (af_p3 - a0_p3 + 3.0 * j_max * (a0 * v0 - af * vf))
            + 6.0 * a0_a0 * a_min * (a_max * a_max - 2.0 * j_max * v0)
            - 6.0 * af_af * a_max * (a_min * a_min - 2.0 * j_max * vf)
            - 12.0 * j_max * (a_max * a_min * (a_max * (v0 + v_max) - a_min * (vf + v_max) - 2.0 * j_max * pd)
                + (a_min - a_max) * j_max * v_max * v_max
                + j_max * (a_max * vf_vf - a_min * v0_v0)))
        / (24.0 * a_max * a_min * j_max_j_max * v_max);
    profile->t[4] = -a_min / j_max;
    profile->t[5] = -(af_af / 2.0 - a_min * a_min - j_max * (vf - v_max)) / (a_min * j_max);
    profile->t[6] = profile->t[4] + af / j_max;
    if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0_ACC1_VEL, false, j_max, v_max, v_min, a_max, a_min)) {
        return true;
    }

    if (a0_a0 / (2.0 * j_max_j_max) + (v_max - v0) / j_max < 0.0) {
        return false;
    }
    t_acc0 = sqrt(a0_a0 / (2.0 * j_max_j_max) + (v_max - v0) / j_max);

    profile->t[0] = t_acc0 - a0 / j_max;
    profile->t[1] = 0.0;
    profile->t[2] = t_acc0;
    profile->t[3] = -(3.0 * af_p4 - 8.0 * a_min * (af_p3 - a0_p3)
            - 24.0 * a_min * j_max * (a0 * v0 - af * vf)
            + 6.0 * af_af * (a_min * a_min - 2.0 * j_max * vf)
            - 12.0 * j_max * (2.0 * a_min * j_max * pd + a_min * a_min * (vf + v_max)
                + j_max * (v_max * v_max - vf_vf)
                + a_min * t_acc0 * (a0_a0 - 2.0 * j_max * (v0 + v_max))))
        / (24.0 * a_min * j_max_j_max * v_max);
    if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC1_VEL, false, j_max, v_max, v_min, a_max, a_min)) {
        return true;
    }

    if (af_af / (2.0 * j_max_j_max) + (v_max - vf) / j_max < 0.0) {
        return false;
    }
    t_acc1 = sqrt(af_af / (2.0 * j_max_j_max) + (v_max - vf) / j_max);

    clear_times(profile);
    profile->t[0] = (-a0 + a_max) / j_max;
    profile->t[1] = (a0_a0 / 2.0 - a_max * a_max - j_max * (v0 - v_max)) / (a_max * j_max);
    profile->t[2] = a_max / j_max;
    profile->t[3] = (3.0 * a0_p4 + 8.0 * a_max * (af_p3 - a0_p3)
            + 24.0 * a_max * j_max * (a0 * v0 - af * vf)
            + 6.0 * a0_a0 * (a_max * a_max - 2.0 * j_max * v0)
            - 12.0 * j_max * (-2.0 * a_max * j_max * pd + a_max * a_max * (v0 + v_max)
                + j_max * (v_max * v_max - v0_v0)
                + a_max * t_acc1 * (-af_af + 2.0 * (vf + v_max) * j_max)))
        / (24.0 * a_max * j_max_j_max * v_max);
    profile->t[4] = t_acc1;
    profile->t[5] = 0.0;
    profile->t[6] = t_acc1 + af / j_max;
    if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0_VEL, false, j_max, v_max, v_min, a_max, a_min)) {
        return true;
    }

    clear_times(profile);
    profile->t[0] = t_acc0 - a0 / j_max;
    profile->t[2] = t_acc0;
    profile->t[3] = (af_p3 - a0_p3) / (3.0 * j_max_j_max * v_max)
        + (a0 * v0 - af * vf + (af_af * t_acc1 + a0_a0 * t_acc0) / 2.0) / (j_max * v_max)
        - (v0 / v_max + 1.0) * t_acc0
        - (vf / v_max + 1.0) * t_acc1
        + pd / v_max;
    profile->t[4] = t_acc1;
    profile->t[6] = t_acc1 + af / j_max;
    return ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_VEL, false, j_max, v_max, v_min, a_max, a_min);
}

static bool time_acc1_vel_two_step(
    ruckig_profile_t* profile,
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
    const double vf_vf = vf * vf;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;

    if (fabs(j_max) < DBL_EPSILON || fabs(v_max) < DBL_EPSILON || fabs(a_min) < DBL_EPSILON) {
        return false;
    }

    clear_times(profile);
    profile->t[2] = a0 / j_max;
    profile->t[3] = -(3.0 * af_p4 - 8.0 * a_min * (af_p3 - a0_p3)
            - 24.0 * a_min * j_max * (a0 * v0 - af * vf)
            + 6.0 * af_af * (a_min * a_min - 2.0 * j_max * vf)
            - 12.0 * j_max * (2.0 * a_min * j_max * pd + a_min * a_min * (vf + v_max)
                + j_max * (v_max * v_max - vf_vf)
                + a_min * a0 * (a0_a0 - 2.0 * j_max * (v0 + v_max)) / j_max))
        / (24.0 * a_min * j_max_j_max * v_max);
    profile->t[4] = -a_min / j_max;
    profile->t[5] = -(af_af / 2.0 - a_min * a_min + j_max * (v_max - vf)) / (a_min * j_max);
    profile->t[6] = profile->t[4] + af / j_max;

    return ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC1_VEL, false, j_max, v_max, v_min, a_max, a_min);
}

static bool time_vel_two_step(
    ruckig_profile_t* profile,
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
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double af_p3 = af_af * af;
    const double j_max_j_max = j_max * j_max;
    double h1;

    if (fabs(j_max) < DBL_EPSILON || fabs(v_max) < DBL_EPSILON) {
        return false;
    }

    h1 = af_af / (2.0 * j_max_j_max) + (v_max - vf) / j_max;
    if (h1 < 0.0) {
        return false;
    }
    h1 = sqrt(h1);

    clear_times(profile);
    profile->t[0] = -a0 / j_max;
    profile->t[3] = (af_p3 - a0_p3) / (3.0 * j_max_j_max * v_max)
        + (a0 * v0 - af * vf + (af_af * h1) / 2.0) / (j_max * v_max)
        - (vf / v_max + 1.0) * h1
        + pd / v_max;
    profile->t[4] = h1;
    profile->t[6] = h1 + af / j_max;

    if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_VEL, false, j_max, v_max, v_min, a_max, a_min)) {
        return true;
    }

    clear_times(profile);
    profile->t[2] = a0 / j_max;
    profile->t[3] = (af_p3 - a0_p3) / (3.0 * j_max_j_max * v_max)
        + (a0 * v0 - af * vf + (af_af * h1 + a0_p3 / j_max) / 2.0) / (j_max * v_max)
        - (v0 / v_max + 1.0) * a0 / j_max
        - (vf / v_max + 1.0) * h1
        + pd / v_max;
    profile->t[4] = h1;
    profile->t[6] = h1 + af / j_max;

    return ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_VEL, false, j_max, v_max, v_min, a_max, a_min);
}

static bool time_none_two_step(
    ruckig_profile_t* profile,
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
    double h0;

    (void)pd;
    if (fabs(j_max) < DBL_EPSILON) {
        return false;
    }

    h0 = (a0 * a0 + af * af) / 2.0 + j_max * (vf - v0);
    if (h0 < 0.0) {
        return false;
    }
    h0 = sqrt(h0) * fabs(j_max) / j_max;

    clear_times(profile);
    profile->t[0] = (h0 - a0) / j_max;
    profile->t[2] = (h0 - af) / j_max;

    if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, false, j_max, v_max, v_min, a_max, a_min)) {
        return true;
    }

    clear_times(profile);
    profile->t[0] = (af - a0) / j_max;

    return ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, false, j_max, v_max, v_min, a_max, a_min);
}

static bool time_acc0_acc1_uddu(
    ruckig_profile_t* profile,
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
    const double v0_v0 = v0 * v0;
    const double vf_vf = vf * vf;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;
    double h1;
    double h2;
    double h3;

    (void)v_min;
    if (fabs(j_max) < DBL_EPSILON || fabs(a_max - a_min) < DBL_EPSILON || fabs(a_max) < DBL_EPSILON || fabs(a_min) < DBL_EPSILON) {
        return false;
    }

    h1 = (3.0 * (af_p4 * a_max - a0_p4 * a_min)
            + a_max * a_min * (8.0 * (a0_p3 - af_p3) + 3.0 * a_max * a_min * (a_max - a_min) + 6.0 * a_min * af_af - 6.0 * a_max * a0_a0)
            + 12.0 * j_max * (a_max * a_min * ((a_max - 2.0 * a0) * v0 - (a_min - 2.0 * af) * vf) + a_min * a0_a0 * v0 - a_max * af_af * vf))
        / (3.0 * (a_max - a_min) * j_max_j_max)
        + 4.0 * (a_max * vf_vf - a_min * v0_v0 - 2.0 * a_min * a_max * pd) / (a_max - a_min);

    if (h1 < 0.0) {
        return false;
    }

    h1 = sqrt(h1) / 2.0;
    h2 = a0_a0 / (2.0 * a_max * j_max) + (a_min - 2.0 * a_max) / (2.0 * j_max) - v0 / a_max;
    h3 = -af_af / (2.0 * a_min * j_max) - (a_max - 2.0 * a_min) / (2.0 * j_max) + vf / a_min;

    if (h2 > h1 / a_max && h3 > -h1 / a_min) {
        clear_times(profile);
        profile->t[0] = (-a0 + a_max) / j_max;
        profile->t[1] = h2 - h1 / a_max;
        profile->t[2] = a_max / j_max;
        profile->t[4] = -a_min / j_max;
        profile->t[5] = h3 + h1 / a_min;
        profile->t[6] = profile->t[4] + af / j_max;

        if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0_ACC1, true, j_max, v_max, v_min, a_max, a_min)) {
            return true;
        }
    }

    if (h2 > -h1 / a_max && h3 > h1 / a_min) {
        clear_times(profile);
        profile->t[0] = (-a0 + a_max) / j_max;
        profile->t[1] = h2 + h1 / a_max;
        profile->t[2] = a_max / j_max;
        profile->t[4] = -a_min / j_max;
        profile->t[5] = h3 - h1 / a_min;
        profile->t[6] = profile->t[4] + af / j_max;

        if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0_ACC1, true, j_max, v_max, v_min, a_max, a_min)) {
            return true;
        }
    }

    return false;
}

static void add_candidate(
    ruckig_profile_t* valid_profiles,
    size_t* valid_profile_count,
    size_t valid_profile_capacity,
    const ruckig_profile_t* candidate
);

static void collect_time_none_uddu(
    const ruckig_profile_t* input,
    ruckig_profile_t* valid_profiles,
    size_t* valid_profile_count,
    size_t valid_profile_capacity,
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
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double af_p3 = af_af * af;
    const double j_max_j_max = j_max * j_max;
    const double h2_none = (a0_a0 - af_af) / (2.0 * j_max) + (vf - v0);
    const double h2_h2 = h2_none * h2_none;
    const double t_min_none = (a0 - af) / j_max;
    const double t_max_none = (a_max - a_min) / j_max;
    ruckig_root_set4_t roots;
    size_t i;

    if (fabs(j_max) < DBL_EPSILON) {
        return;
    }

    roots = ruckig_solve_quart_monic(
        0.0,
        -2.0 * (a0_a0 + af_af - 2.0 * j_max * (v0 + vf)) / j_max_j_max,
        4.0 * (a0_p3 - af_p3 + 3.0 * j_max * (af * vf - a0 * v0)) / (3.0 * j_max * j_max_j_max) - 4.0 * pd / j_max,
        -h2_h2 / j_max_j_max
    );

    for (i = 0; i < roots.count; ++i) {
        ruckig_profile_t candidate;
        double t = roots.values[i];
        double h0;

        if (t < t_min_none || t > t_max_none) {
            continue;
        }

        if (t > DBL_EPSILON) {
            const double h1 = j_max * t * t;
            const double orig = -h2_h2 / (4.0 * j_max * t)
                + h2_none * (af / j_max + t)
                + (4.0 * a0_p3 + 2.0 * af_p3 - 6.0 * a0_a0 * (af + 2.0 * j_max * t)
                    + 12.0 * (af - a0) * j_max * v0
                    + 3.0 * j_max_j_max * (-4.0 * pd + (h1 + 8.0 * v0) * t))
                    / (12.0 * j_max_j_max);
            const double deriv = h2_none + 2.0 * v0 - a0_a0 / j_max + h2_h2 / (4.0 * h1) + (3.0 * h1) / 4.0;
            t -= orig / deriv;
        }

        h0 = h2_none / (2.0 * j_max * t);
        ruckig_profile_init(&candidate);
        ruckig_profile_copy_boundary(&candidate, input);
        candidate.t[0] = h0 + t / 2.0 - a0 / j_max;
        candidate.t[2] = t;
        candidate.t[6] = -h0 + t / 2.0 + af / j_max;

        if (ruckig_profile_check(&candidate, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, false, j_max, v_max, v_min, a_max, a_min)) {
            add_candidate(valid_profiles, valid_profile_count, valid_profile_capacity, &candidate);
        }
    }
}

static bool time_acc1_uddu(
    ruckig_profile_t* profile,
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
    const double vf_vf = vf * vf;
    const double v0_v0 = v0 * v0;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;
    const double h3_acc1 = -(a0_a0 + af_af) / (2.0 * j_max * a_min) + a_min / j_max + (vf - v0) / a_min;
    const double t_min_acc1 = (a_min - a0) / j_max;
    const double t_max_acc1 = (a_max - a0) / j_max;
    const double h0_acc1 = (a0_p4 - af_p4) / 4.0 + 2.0 * (af_p3 - a0_p3) * a_min / 3.0
        + (a0_a0 - af_af) * a_min * a_min / 2.0
        + j_max * (af_af * vf + a0_a0 * v0 + 2.0 * a_min * (j_max * pd - a0 * v0 - af * vf)
            + a_min * a_min * (v0 + vf) + j_max * (v0_v0 - vf_vf));
    const double h2_acc1 = a0_a0 - a0 * a_min + 2.0 * j_max * v0;
    ruckig_root_set4_t roots;
    size_t i;

    if (fabs(j_max) < DBL_EPSILON || fabs(a_min) < DBL_EPSILON) {
        return false;
    }

    roots = ruckig_solve_quart_monic(
        2.0 * (2.0 * a0 - a_min) / j_max,
        (5.0 * a0_a0 + a_min * (a_min - 6.0 * a0) + 2.0 * j_max * v0) / j_max_j_max,
        2.0 * (a0 - a_min) * h2_acc1 / (j_max_j_max * j_max),
        h0_acc1 / (j_max_j_max * j_max_j_max)
    );

    for (i = 0; i < roots.count; ++i) {
        double t = roots.values[i];

        if (t < t_min_acc1 || t > t_max_acc1) {
            continue;
        }

        if (t > DBL_EPSILON) {
            const double h5 = a0_p3 + 2.0 * j_max * a0 * v0;
            double h1 = j_max * t;
            double orig = -(h0_acc1 / 2.0 + h1 * (h5 + a0 * (a_min - 2.0 * h1) * (a_min - h1)
                    + a0_a0 * (5.0 * h1 / 2.0 - 2.0 * a_min) + a_min * a_min * h1 / 2.0
                    + j_max * (h1 / 2.0 - a_min) * (h1 * t + 2.0 * v0))) / j_max;
            double deriv = (a_min - a0 - h1) * (h2_acc1 + h1 * (4.0 * a0 - a_min + 2.0 * h1));
            if (fabs(deriv) > DBL_EPSILON) {
                const double step = orig / deriv;
                t -= step < t ? step : t;
            }

            h1 = j_max * t;
            orig = -(h0_acc1 / 2.0 + h1 * (h5 + a0 * (a_min - 2.0 * h1) * (a_min - h1)
                    + a0_a0 * (5.0 * h1 / 2.0 - 2.0 * a_min) + a_min * a_min * h1 / 2.0
                    + j_max * (h1 / 2.0 - a_min) * (h1 * t + 2.0 * v0))) / j_max;

            if (fabs(orig) > 1e-9) {
                deriv = (a_min - a0 - h1) * (h2_acc1 + h1 * (4.0 * a0 - a_min + 2.0 * h1));
                if (fabs(deriv) > DBL_EPSILON) {
                    t -= orig / deriv;
                }

                h1 = j_max * t;
                orig = -(h0_acc1 / 2.0 + h1 * (h5 + a0 * (a_min - 2.0 * h1) * (a_min - h1)
                        + a0_a0 * (5.0 * h1 / 2.0 - 2.0 * a_min) + a_min * a_min * h1 / 2.0
                        + j_max * (h1 / 2.0 - a_min) * (h1 * t + 2.0 * v0))) / j_max;

                if (fabs(orig) > 1e-9) {
                    deriv = (a_min - a0 - h1) * (h2_acc1 + h1 * (4.0 * a0 - a_min + 2.0 * h1));
                    if (fabs(deriv) > DBL_EPSILON) {
                        t -= orig / deriv;
                    }
                }
            }
        }

        clear_times(profile);
        profile->t[0] = t;
        profile->t[2] = (a0 - a_min) / j_max + t;
        profile->t[5] = h3_acc1 - (2.0 * a0 + j_max * t) * t / a_min;
        profile->t[6] = (af - a_min) / j_max;

        if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC1, true, j_max, v_max, v_min, a_max, a_min)) {
            return true;
        }
    }

    return false;
}

static bool time_acc0_general_uddu(
    ruckig_profile_t* profile,
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
    const double v0_v0 = v0 * v0;
    const double vf_vf = vf * vf;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;
    const double h3_acc0 = (a0_a0 - af_af) / (2.0 * a_max * j_max) + (vf - v0) / a_max;
    const double t_min_acc0 = (a_max - af) / j_max;
    const double t_max_acc0 = (a_max - a_min) / j_max;
    const double h0_acc0 = 3.0 * (af_p4 - a0_p4) + 8.0 * (a0_p3 - af_p3) * a_max
        + 24.0 * a_max * j_max * (af * vf - a0 * v0)
        - 6.0 * a0_a0 * (a_max * a_max - 2.0 * j_max * v0)
        + 6.0 * af_af * (a_max * a_max - 2.0 * j_max * vf)
        + 12.0 * j_max * (j_max * (vf_vf - v0_v0 - 2.0 * a_max * pd) - a_max * a_max * (vf - v0));
    const double h2_acc0 = -af_af + a_max * a_max + 2.0 * j_max * vf;
    ruckig_root_set4_t roots;
    size_t i;

    if (fabs(j_max) < DBL_EPSILON || fabs(a_max) < DBL_EPSILON) {
        return false;
    }

    roots = ruckig_solve_quart_monic(
        -2.0 * a_max / j_max,
        h2_acc0 / j_max_j_max,
        0.0,
        h0_acc0 / (12.0 * j_max_j_max * j_max_j_max)
    );

    for (i = 0; i < roots.count; ++i) {
        double t = roots.values[i];

        if (t < t_min_acc0 || t > t_max_acc0) {
            continue;
        }

        if (t > DBL_EPSILON) {
            const double h1 = j_max * t;
            const double orig = h0_acc0 / (12.0 * j_max_j_max * t) + t * (h2_acc0 + h1 * (h1 - 2.0 * a_max));
            const double deriv = 2.0 * (h2_acc0 + h1 * (2.0 * h1 - 3.0 * a_max));
            if (fabs(deriv) > DBL_EPSILON) {
                /* Refine the algebraic root before profile checks, which remain the acceptance gate. */
                t -= orig / deriv;
            }
        }

        clear_times(profile);
        profile->t[0] = (-a0 + a_max) / j_max;
        profile->t[1] = h3_acc0 - 2.0 * t + j_max / a_max * t * t;
        profile->t[2] = t;
        profile->t[6] = (af - a_max) / j_max + t;

        if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, true, j_max, v_max, v_min, a_max, a_min)) {
            return true;
        }
    }

    return false;
}

static bool time_acc0_uddu(
    ruckig_profile_t* profile,
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
    const double v0_v0 = v0 * v0;
    const double vf_vf = vf * vf;
    const double a0_a0 = a0 * a0;
    const double af_af = af * af;
    const double a0_p3 = a0_a0 * a0;
    const double a0_p4 = a0_a0 * a0_a0;
    const double af_p3 = af_af * af;
    const double af_p4 = af_af * af_af;
    const double j_max_j_max = j_max * j_max;

    if (fabs(j_max) < DBL_EPSILON) {
        return false;
    }

    if (fabs(a0) > DBL_EPSILON) {
        clear_times(profile);
        profile->t[1] = (af_af - a0_a0 + 2.0 * j_max * (vf - v0)) / (2.0 * a0 * j_max);
        profile->t[2] = (a0 - af) / j_max;

        if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, true, j_max, v_max, v_min, a_max, a_min)) {
            return true;
        }
    }

    if (fabs(a_max) > DBL_EPSILON) {
        clear_times(profile);
        profile->t[0] = (-a0 + a_max) / j_max;
        profile->t[1] = (a0_a0 + af_af - 2.0 * a_max * a_max + 2.0 * j_max * (vf - v0)) / (2.0 * a_max * j_max);
        profile->t[2] = (-af + a_max) / j_max;

        if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, true, j_max, v_max, v_min, a_max, a_min)) {
            return true;
        }
    }

    {
        const double h0 = 3.0 * (af_af - a0_a0 + 2.0 * j_max * (v0 + vf));
        if (fabs(h0) > DBL_EPSILON) {
            const double h2 = a0_p3 + 2.0 * af_p3 + 6.0 * j_max_j_max * pd + 6.0 * (af - a0) * j_max * vf - 3.0 * a0 * af_af;
            const double radicand = 2.0 * (2.0 * h2 * h2
                + h0 * (a0_p4 - 6.0 * a0_a0 * (af_af + 2.0 * j_max * vf)
                    + 8.0 * a0 * (af_p3 + 3.0 * j_max_j_max * pd + 3.0 * af * j_max * vf)
                    - 3.0 * (af_p4 + 4.0 * af_af * j_max * vf + 4.0 * j_max_j_max * (vf_vf - v0_v0))));
            if (radicand >= 0.0) {
                const double h1 = sqrt(radicand) * fabs(j_max) / j_max;
                clear_times(profile);
                profile->t[0] = (4.0 * af_p3 + 2.0 * a0_p3 - 6.0 * a0 * af_af + 12.0 * j_max_j_max * pd + 12.0 * (af - a0) * j_max * vf + h1) / (2.0 * j_max * h0);
                profile->t[1] = -h1 / (j_max * h0);
                profile->t[2] = (-4.0 * a0_p3 - 2.0 * af_p3 + 6.0 * a0_a0 * af + 12.0 * j_max_j_max * pd - 12.0 * (af - a0) * j_max * v0 + h1) / (2.0 * j_max * h0);

                if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, true, j_max, v_max, v_min, a_max, a_min)) {
                    return true;
                }
            }
        }
    }

    {
        const double t = (a_max - a_min) / j_max;
        if (fabs(a_max) > DBL_EPSILON) {
            clear_times(profile);
            profile->t[0] = (-a0 + a_max) / j_max;
            profile->t[1] = (a0_a0 - af_af) / (2.0 * a_max * j_max) + (vf - v0 + j_max * t * t) / a_max - 2.0 * t;
            profile->t[2] = t;
            profile->t[6] = (af - a_min) / j_max;

            if (ruckig_profile_check(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, true, j_max, v_max, v_min, a_max, a_min)) {
                return true;
            }
        }
    }

    return false;
}

static void add_candidate(
    ruckig_profile_t* valid_profiles,
    size_t* valid_profile_count,
    size_t valid_profile_capacity,
    const ruckig_profile_t* candidate
) {
    if (*valid_profile_count < valid_profile_capacity) {
        valid_profiles[*valid_profile_count] = *candidate;
        *valid_profile_count += 1;
    }
}

static void try_add_family(
    const ruckig_profile_t* input,
    ruckig_profile_t* valid_profiles,
    size_t* valid_profile_count,
    size_t valid_profile_capacity,
    bool (*family)(
        ruckig_profile_t*,
        double,
        double,
        double,
        double,
        double,
        double,
        double,
        double,
        double,
        double
    ),
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
    ruckig_profile_t candidate;
    ruckig_profile_init(&candidate);
    ruckig_profile_copy_boundary(&candidate, input);
    if (family(&candidate, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max)) {
        add_candidate(valid_profiles, valid_profile_count, valid_profile_capacity, &candidate);
    }
}

static bool collect_position_third_step1_all_none_acc0_acc1(
    const ruckig_profile_t* input,
    ruckig_profile_t* valid_profiles,
    size_t* valid_profile_count,
    size_t valid_profile_capacity,
    double pd,
    double v0,
    double a0,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max,
    bool return_after_found
) {
    const size_t start_count = *valid_profile_count;

    collect_time_none_uddu(input, valid_profiles, valid_profile_count, valid_profile_capacity, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max);
    if (return_after_found && *valid_profile_count > start_count) {
        return true;
    }

    try_add_family(input, valid_profiles, valid_profile_count, valid_profile_capacity, time_acc0_general_uddu, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max);
    if (return_after_found && *valid_profile_count > start_count) {
        return true;
    }

    try_add_family(input, valid_profiles, valid_profile_count, valid_profile_capacity, time_acc1_uddu, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max);
    if (return_after_found && *valid_profile_count > start_count) {
        return true;
    }

    return *valid_profile_count > start_count;
}

bool ruckig_position_third_step1_get_profile(
    const ruckig_profile_t* input,
    ruckig_profile_t* output,
    ruckig_block_t* block,
    double* duration,
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
    const bool up_first = pd >= 0.0;
    const double oriented_v_max = up_first ? v_max : v_min;
    const double oriented_v_min = up_first ? v_min : v_max;
    const double oriented_a_max = up_first ? a_max : a_min;
    const double oriented_a_min = up_first ? a_min : a_max;
    const double oriented_j_max = up_first ? j_max : -j_max;
    const double reverse_v_max = up_first ? v_min : v_max;
    const double reverse_v_min = up_first ? v_max : v_min;
    const double reverse_a_max = up_first ? a_min : a_max;
    const double reverse_a_min = up_first ? a_max : a_min;
    const double reverse_j_max = up_first ? -j_max : j_max;
    ruckig_profile_t valid_profiles[8];
    size_t valid_profile_count = 0;

    if (!input || !output || !block || !duration) {
        return false;
    }

    ruckig_block_init(block);
    ruckig_profile_copy_boundary(output, input);

    if (j_max == 0.0 || a_max == 0.0 || a_min == 0.0) {
        if (!time_all_single_step(output, pd, v0, a0, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min)) {
            return false;
        }
        block->p_min = *output;
        block->t_min = output->t_sum[6] + output->brake.duration + output->accel.duration;
        block->valid = true;
        if (fabs(v0) > DBL_EPSILON || fabs(a0) > DBL_EPSILON) {
            block->a.valid = true;
            block->a.left = block->t_min;
            block->a.right = INFINITY;
            block->a.profile = *output;
        }
    } else {
        if (fabs(vf) < DBL_EPSILON && fabs(af) < DBL_EPSILON) {
            if (fabs(v0) < DBL_EPSILON && fabs(a0) < DBL_EPSILON && fabs(pd) < DBL_EPSILON) {
                collect_position_third_step1_all_none_acc0_acc1(input, valid_profiles, &valid_profile_count, 6, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_max, true);
            } else {
                try_add_family(input, valid_profiles, &valid_profile_count, 6, time_symmetric_rest_to_rest, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_max);
                if (valid_profile_count == 0) {
                    try_add_family(input, valid_profiles, &valid_profile_count, 6, time_vel_rest_to_rest, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_max);
                }
                try_add_family(input, valid_profiles, &valid_profile_count, 6, time_all_vel_uddu, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_max);
                if (valid_profile_count == 0) {
                    collect_position_third_step1_all_none_acc0_acc1(input, valid_profiles, &valid_profile_count, 6, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_max, true);
                }
                if (valid_profile_count == 0) {
                    try_add_family(input, valid_profiles, &valid_profile_count, 6, time_acc0_acc1_uddu, pd, v0, a0, vf, af, oriented_v_max, oriented_v_min, oriented_a_max, oriented_a_min, oriented_j_max);
                }
                if (valid_profile_count == 0) {
                    try_add_family(input, valid_profiles, &valid_profile_count, 6, time_all_vel_uddu, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_max);
                }
                if (valid_profile_count == 0) {
                    collect_position_third_step1_all_none_acc0_acc1(input, valid_profiles, &valid_profile_count, 6, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_max, true);
                }
                if (valid_profile_count == 0) {
                    try_add_family(input, valid_profiles, &valid_profile_count, 6, time_acc0_acc1_uddu, pd, v0, a0, vf, af, reverse_v_max, reverse_v_min, reverse_a_max, reverse_a_min, reverse_j_max);
                }
            }
        } else {
            collect_position_third_step1_all_none_acc0_acc1(input, valid_profiles, &valid_profile_count, 6, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max, false);
            collect_position_third_step1_all_none_acc0_acc1(input, valid_profiles, &valid_profile_count, 6, pd, v0, a0, vf, af, v_min, v_max, a_min, a_max, -j_max, false);
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_acc0_acc1_uddu, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max);
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_acc0_acc1_uddu, pd, v0, a0, vf, af, v_min, v_max, a_min, a_max, -j_max);
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_all_vel_uddu, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max);
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_all_vel_uddu, pd, v0, a0, vf, af, v_min, v_max, a_min, a_max, -j_max);
        }

        if (valid_profile_count == 0) {
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_none_two_step, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max);
        }
        if (valid_profile_count == 0) {
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_none_two_step, pd, v0, a0, vf, af, v_min, v_max, a_min, a_max, -j_max);
        }
        if (valid_profile_count == 0) {
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_acc0_uddu, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max);
        }
        if (valid_profile_count == 0) {
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_acc0_uddu, pd, v0, a0, vf, af, v_min, v_max, a_min, a_max, -j_max);
        }
        if (valid_profile_count == 0) {
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_vel_two_step, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max);
        }
        if (valid_profile_count == 0) {
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_vel_two_step, pd, v0, a0, vf, af, v_min, v_max, a_min, a_max, -j_max);
        }
        if (valid_profile_count == 0) {
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_acc1_vel_two_step, pd, v0, a0, vf, af, v_max, v_min, a_max, a_min, j_max);
        }
        if (valid_profile_count == 0) {
            try_add_family(input, valid_profiles, &valid_profile_count, 6, time_acc1_vel_two_step, pd, v0, a0, vf, af, v_min, v_max, a_min, a_max, -j_max);
        }

        if (valid_profile_count == 0 || !ruckig_block_calculate(block, valid_profiles, valid_profile_count)) {
            return false;
        }
        *output = block->p_min;
    }

    *duration = output->t_sum[6] + output->brake.duration + output->accel.duration;
    return true;
}
