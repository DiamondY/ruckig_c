#include "ruckig_c/profile.h"

#include "ruckig_c/precision.h"
#include "ruckig_c/roots.h"
#include "ruckig_c/utils.h"

#include <float.h>
#include <math.h>
#include <string.h>

static const double profile_v_eps = RUCKIG_C_PROFILE_V_EPS;
static const double profile_a_eps = RUCKIG_C_PROFILE_A_EPS;
static const double profile_j_eps = RUCKIG_C_PROFILE_J_EPS;
static const double profile_p_precision = RUCKIG_C_PROFILE_P_PRECISION;
static const double profile_v_precision = RUCKIG_C_PROFILE_V_PRECISION;
static const double profile_a_precision = RUCKIG_C_PROFILE_A_PRECISION;
static const double profile_first_position_precision = RUCKIG_C_PROFILE_FIRST_POSITION_PRECISION;

void ruckig_profile_init(ruckig_profile_t* profile) {
    if (profile) {
        memset(profile, 0, sizeof(*profile));
        profile->limits = RUCKIG_PROFILE_LIMITS_NONE;
        profile->direction = RUCKIG_PROFILE_DIRECTION_UP;
        profile->control_signs = RUCKIG_PROFILE_SIGNS_UDDU;
    }
}

void ruckig_profile_set_boundary(ruckig_profile_t* profile, double p0, double v0, double a0, double pf, double vf, double af) {
    if (!profile) {
        return;
    }
    profile->a[0] = a0;
    profile->v[0] = v0;
    profile->p[0] = p0;
    profile->af = af;
    profile->vf = vf;
    profile->pf = pf;
}

void ruckig_profile_set_boundary_for_velocity(ruckig_profile_t* profile, double p0, double v0, double a0, double vf, double af) {
    if (!profile) {
        return;
    }
    profile->a[0] = a0;
    profile->v[0] = v0;
    profile->p[0] = p0;
    profile->af = af;
    profile->vf = vf;
}

void ruckig_profile_copy_boundary(ruckig_profile_t* profile, const ruckig_profile_t* source) {
    if (!profile || !source) {
        return;
    }
    profile->a[0] = source->a[0];
    profile->v[0] = source->v[0];
    profile->p[0] = source->p[0];
    profile->af = source->af;
    profile->vf = source->vf;
    profile->pf = source->pf;
    profile->brake = source->brake;
    profile->accel = source->accel;
}

static bool calculate_t_sum(ruckig_profile_t* profile) {
    size_t i;
    if (profile->t[0] < 0.0) {
        return false;
    }
    profile->t_sum[0] = profile->t[0];
    for (i = 0; i < 6; ++i) {
        if (profile->t[i + 1] < 0.0) {
            return false;
        }
        profile->t_sum[i + 1] = profile->t_sum[i] + profile->t[i + 1];
    }
    /* Reject profiles whose accumulated duration is numerically valid but
       not usable by the local solver. */
    return profile->t_sum[6] <= RUCKIG_C_PROFILE_T_MAX;
}

static bool limits_require_velocity_segment(ruckig_profile_reached_limits_t limits) {
    return limits == RUCKIG_PROFILE_LIMITS_ACC0_ACC1_VEL
        || limits == RUCKIG_PROFILE_LIMITS_ACC0_VEL
        || limits == RUCKIG_PROFILE_LIMITS_ACC1_VEL
        || limits == RUCKIG_PROFILE_LIMITS_VEL;
}

static bool limits_require_acc0_segment(ruckig_profile_reached_limits_t limits) {
    return limits == RUCKIG_PROFILE_LIMITS_ACC0
        || limits == RUCKIG_PROFILE_LIMITS_ACC0_ACC1;
}

static bool limits_require_acc1_segment(ruckig_profile_reached_limits_t limits) {
    return limits == RUCKIG_PROFILE_LIMITS_ACC1
        || limits == RUCKIG_PROFILE_LIMITS_ACC0_ACC1;
}

static bool limits_force_a3_zero(ruckig_profile_reached_limits_t limits) {
    return limits == RUCKIG_PROFILE_LIMITS_ACC0_ACC1_VEL
        || limits == RUCKIG_PROFILE_LIMITS_ACC0_ACC1
        || limits == RUCKIG_PROFILE_LIMITS_ACC0_VEL
        || limits == RUCKIG_PROFILE_LIMITS_ACC1_VEL
        || limits == RUCKIG_PROFILE_LIMITS_VEL;
}

static void assign_jerk(ruckig_profile_t* profile, ruckig_profile_control_signs_t signs, double jf) {
    profile->j[0] = profile->t[0] > 0.0 ? jf : 0.0;
    profile->j[1] = 0.0;
    profile->j[2] = profile->t[2] > 0.0 ? -jf : 0.0;
    profile->j[3] = 0.0;
    if (signs == RUCKIG_PROFILE_SIGNS_UDDU) {
        profile->j[4] = profile->t[4] > 0.0 ? -jf : 0.0;
        profile->j[6] = profile->t[6] > 0.0 ? jf : 0.0;
    } else {
        profile->j[4] = profile->t[4] > 0.0 ? jf : 0.0;
        profile->j[6] = profile->t[6] > 0.0 ? -jf : 0.0;
    }
    profile->j[5] = 0.0;
}

static void integrate_third_order_segments(ruckig_profile_t* profile) {
    size_t i;
    for (i = 0; i < 7; ++i) {
        profile->a[i + 1] = profile->a[i] + profile->t[i] * profile->j[i];
        profile->v[i + 1] = profile->v[i] + profile->t[i] * (profile->a[i] + profile->t[i] * profile->j[i] / 2.0);
        profile->p[i + 1] = profile->p[i] + profile->t[i] * (profile->v[i] + profile->t[i] * (profile->a[i] / 2.0 + profile->t[i] * profile->j[i] / 6.0));
    }
}

bool ruckig_profile_check_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_third_order_check_t* check
) {
    size_t i;
    ruckig_profile_control_signs_t signs;
    ruckig_profile_reached_limits_t limits;
    double v_upp_lim;
    double v_low_lim;
    double a_upp_lim;
    double a_low_lim;
    bool set_limits;
    double jf;
    double v_max;
    double v_min;
    double a_max;
    double a_min;

    if (!profile || !check || !calculate_t_sum(profile)) {
        return false;
    }
    signs = check->signs;
    limits = check->limits;
    set_limits = check->set_limits;
    jf = check->jf;
    v_max = check->v_max;
    v_min = check->v_min;
    a_max = check->a_max;
    a_min = check->a_min;
    if (limits_require_velocity_segment(limits) && profile->t[3] < DBL_EPSILON) {
        return false;
    }
    if (limits_require_acc0_segment(limits) && profile->t[1] < DBL_EPSILON) {
        return false;
    }
    if (limits_require_acc1_segment(limits) && profile->t[5] < DBL_EPSILON) {
        return false;
    }

    assign_jerk(profile, signs, jf);
    profile->direction = v_max > 0.0 ? RUCKIG_PROFILE_DIRECTION_UP : RUCKIG_PROFILE_DIRECTION_DOWN;
    v_upp_lim = (profile->direction == RUCKIG_PROFILE_DIRECTION_UP ? v_max : v_min) + profile_v_eps;
    v_low_lim = (profile->direction == RUCKIG_PROFILE_DIRECTION_UP ? v_min : v_max) - profile_v_eps;

    for (i = 0; i < 7; ++i) {
        profile->a[i + 1] = profile->a[i] + profile->t[i] * profile->j[i];
        profile->v[i + 1] = profile->v[i] + profile->t[i] * (profile->a[i] + profile->t[i] * profile->j[i] / 2.0);
        profile->p[i + 1] = profile->p[i] + profile->t[i] * (profile->v[i] + profile->t[i] * (profile->a[i] / 2.0 + profile->t[i] * profile->j[i] / 6.0));

        if (limits_force_a3_zero(limits) && i == 2) {
            profile->a[3] = 0.0;
        }

        if (set_limits) {
            if (limits == RUCKIG_PROFILE_LIMITS_ACC1 && i == 2) {
                profile->a[3] = a_min;
            }
            if (limits == RUCKIG_PROFILE_LIMITS_ACC0_ACC1) {
                if (i == 0) {
                    profile->a[1] = a_max;
                }
                if (i == 4) {
                    profile->a[5] = a_min;
                }
            }
        }

        if (i > 1 && profile->a[i + 1] * profile->a[i] < -DBL_EPSILON) {
            const double v_a_zero = profile->v[i] - (profile->a[i] * profile->a[i]) / (2.0 * profile->j[i]);
            if (v_a_zero > v_upp_lim || v_a_zero < v_low_lim) {
                return false;
            }
        }
    }

    profile->control_signs = signs;
    profile->limits = limits;
    a_upp_lim = (profile->direction == RUCKIG_PROFILE_DIRECTION_UP ? a_max : a_min) + profile_a_eps;
    a_low_lim = (profile->direction == RUCKIG_PROFILE_DIRECTION_UP ? a_min : a_max) - profile_a_eps;

    return fabs(profile->p[7] - profile->pf) < profile_p_precision
        && fabs(profile->v[7] - profile->vf) < profile_v_precision
        && fabs(profile->a[7] - profile->af) < profile_a_precision
        && profile->a[1] >= a_low_lim && profile->a[3] >= a_low_lim && profile->a[5] >= a_low_lim
        && profile->a[1] <= a_upp_lim && profile->a[3] <= a_upp_lim && profile->a[5] <= a_upp_lim
        && profile->v[3] <= v_upp_lim && profile->v[4] <= v_upp_lim && profile->v[5] <= v_upp_lim && profile->v[6] <= v_upp_lim
        && profile->v[3] >= v_low_lim && profile->v[4] >= v_low_lim && profile->v[5] >= v_low_lim && profile->v[6] >= v_low_lim;
}

bool ruckig_profile_check_with_timing_ctx(ruckig_profile_t* profile, const ruckig_profile_third_order_check_t* check) {
    ruckig_profile_third_order_check_t timing_check;
    if (!check) {
        return false;
    }
    timing_check = *check;
    timing_check.set_limits = false;
    (void)timing_check.tf;
    return ruckig_profile_check_ctx(profile, &timing_check);
}

bool ruckig_profile_check_with_timing_guarded_ctx(ruckig_profile_t* profile, const ruckig_profile_third_order_check_t* check) {
    return check
        && fabs(check->jf) < fabs(check->j_max) + profile_j_eps
        && ruckig_profile_check_with_timing_ctx(profile, check);
}

bool ruckig_profile_check_for_velocity_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_third_order_velocity_check_t* check
) {
    double a_upp_lim;
    double a_low_lim;
    ruckig_profile_control_signs_t signs;
    ruckig_profile_reached_limits_t limits;
    double jf;
    double a_max;
    double a_min;

    if (!profile || !check || !calculate_t_sum(profile)) {
        return false;
    }
    signs = check->signs;
    limits = check->limits;
    jf = check->jf;
    a_max = check->a_max;
    a_min = check->a_min;
    if (limits == RUCKIG_PROFILE_LIMITS_ACC0 && profile->t[1] < DBL_EPSILON) {
        return false;
    }

    assign_jerk(profile, signs, jf);
    integrate_third_order_segments(profile);
    profile->control_signs = signs;
    profile->limits = limits;
    profile->direction = a_max > 0.0 ? RUCKIG_PROFILE_DIRECTION_UP : RUCKIG_PROFILE_DIRECTION_DOWN;
    a_upp_lim = (profile->direction == RUCKIG_PROFILE_DIRECTION_UP ? a_max : a_min) + profile_a_eps;
    a_low_lim = (profile->direction == RUCKIG_PROFILE_DIRECTION_UP ? a_min : a_max) - profile_a_eps;

    return fabs(profile->v[7] - profile->vf) < profile_v_precision
        && fabs(profile->a[7] - profile->af) < profile_a_precision
        && profile->a[1] >= a_low_lim && profile->a[3] >= a_low_lim && profile->a[5] >= a_low_lim
        && profile->a[1] <= a_upp_lim && profile->a[3] <= a_upp_lim && profile->a[5] <= a_upp_lim;
}

bool ruckig_profile_check_for_velocity_with_timing_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_third_order_velocity_check_t* check
) {
    if (check) {
        (void)check->tf;
    }
    return ruckig_profile_check_for_velocity_ctx(profile, check);
}

bool ruckig_profile_check_for_velocity_with_timing_guarded_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_third_order_velocity_check_t* check
) {
    return check
        && fabs(check->jf) < fabs(check->j_max) + profile_j_eps
        && ruckig_profile_check_for_velocity_with_timing_ctx(profile, check);
}

bool ruckig_profile_check_for_second_order_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_second_order_check_t* check
) {
    size_t i;
    ruckig_profile_control_signs_t signs;
    ruckig_profile_reached_limits_t limits;
    double v_upp_lim;
    double v_low_lim;
    double a_up;
    double a_down;
    double v_max;
    double v_min;

    if (!profile || !check || !calculate_t_sum(profile)) {
        return false;
    }
    signs = check->signs;
    limits = check->limits;
    a_up = check->a_up;
    a_down = check->a_down;
    v_max = check->v_max;
    v_min = check->v_min;

    memset(profile->j, 0, sizeof(profile->j));
    if (signs == RUCKIG_PROFILE_SIGNS_UDDU) {
        profile->a[0] = profile->t[0] > 0.0 ? a_up : 0.0;
        profile->a[2] = profile->t[2] > 0.0 ? a_down : 0.0;
        profile->a[4] = profile->t[4] > 0.0 ? a_down : 0.0;
        profile->a[6] = profile->t[6] > 0.0 ? a_up : 0.0;
    } else {
        profile->a[0] = profile->t[0] > 0.0 ? a_up : 0.0;
        profile->a[2] = profile->t[2] > 0.0 ? a_down : 0.0;
        profile->a[4] = profile->t[4] > 0.0 ? a_up : 0.0;
        profile->a[6] = profile->t[6] > 0.0 ? a_down : 0.0;
    }
    profile->a[1] = 0.0;
    profile->a[3] = 0.0;
    profile->a[5] = 0.0;
    profile->a[7] = profile->af;

    profile->direction = v_max > 0.0 ? RUCKIG_PROFILE_DIRECTION_UP : RUCKIG_PROFILE_DIRECTION_DOWN;
    v_upp_lim = (profile->direction == RUCKIG_PROFILE_DIRECTION_UP ? v_max : v_min) + profile_v_eps;
    v_low_lim = (profile->direction == RUCKIG_PROFILE_DIRECTION_UP ? v_min : v_max) - profile_v_eps;

    for (i = 0; i < 7; ++i) {
        profile->v[i + 1] = profile->v[i] + profile->t[i] * profile->a[i];
        profile->p[i + 1] = profile->p[i] + profile->t[i] * (profile->v[i] + profile->t[i] * profile->a[i] / 2.0);
    }

    profile->control_signs = signs;
    profile->limits = limits;

    return fabs(profile->p[7] - profile->pf) < profile_p_precision
        && fabs(profile->v[7] - profile->vf) < profile_v_precision
        && profile->v[2] <= v_upp_lim && profile->v[3] <= v_upp_lim && profile->v[4] <= v_upp_lim && profile->v[5] <= v_upp_lim && profile->v[6] <= v_upp_lim
        && profile->v[2] >= v_low_lim && profile->v[3] >= v_low_lim && profile->v[4] >= v_low_lim && profile->v[5] >= v_low_lim && profile->v[6] >= v_low_lim;
}

bool ruckig_profile_check_for_second_order_with_timing_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_second_order_check_t* check
) {
    if (check) {
        (void)check->tf;
    }
    return ruckig_profile_check_for_second_order_ctx(profile, check);
}

bool ruckig_profile_check_for_second_order_with_timing_guarded_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_second_order_check_t* check
) {
    return check
        && (check->a_min - profile_a_eps < check->a_up)
        && (check->a_up < check->a_max + profile_a_eps)
        && (check->a_min - profile_a_eps < check->a_down)
        && (check->a_down < check->a_max + profile_a_eps)
        && ruckig_profile_check_for_second_order_with_timing_ctx(profile, check);
}

bool ruckig_profile_check_for_second_order_velocity_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_second_order_velocity_check_t* check
) {
    size_t i;
    ruckig_profile_control_signs_t signs;
    ruckig_profile_reached_limits_t limits;
    double a_up;
    if (!profile || !check || profile->t[1] < 0.0) {
        return false;
    }
    signs = check->signs;
    limits = check->limits;
    a_up = check->a_up;

    /* Second-order velocity profiles are single-segment constant-acceleration profiles;
       later t_sum slots intentionally share the same end time. */
    profile->t_sum[0] = 0.0;
    profile->t_sum[1] = profile->t[1];
    profile->t_sum[2] = profile->t[1];
    profile->t_sum[3] = profile->t[1];
    profile->t_sum[4] = profile->t[1];
    profile->t_sum[5] = profile->t[1];
    profile->t_sum[6] = profile->t[1];
    if (profile->t_sum[6] > RUCKIG_C_PROFILE_T_MAX) {
        return false;
    }

    memset(profile->j, 0, sizeof(profile->j));
    memset(profile->a, 0, sizeof(profile->a));
    profile->a[1] = profile->t[1] > 0.0 ? a_up : 0.0;
    profile->a[7] = profile->af;
    for (i = 0; i < 7; ++i) {
        profile->v[i + 1] = profile->v[i] + profile->t[i] * profile->a[i];
        profile->p[i + 1] = profile->p[i] + profile->t[i] * (profile->v[i] + profile->t[i] * profile->a[i] / 2.0);
    }

    profile->control_signs = signs;
    profile->limits = limits;
    profile->direction = a_up > 0.0 ? RUCKIG_PROFILE_DIRECTION_UP : RUCKIG_PROFILE_DIRECTION_DOWN;
    return fabs(profile->v[7] - profile->vf) < profile_v_precision;
}

bool ruckig_profile_check_for_second_order_velocity_with_timing_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_second_order_velocity_check_t* check
) {
    if (check) {
        (void)check->tf;
    }
    return ruckig_profile_check_for_second_order_velocity_ctx(profile, check);
}

bool ruckig_profile_check_for_second_order_velocity_with_timing_guarded_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_second_order_velocity_check_t* check
) {
    return check
        && (check->a_min - profile_a_eps < check->a_up)
        && (check->a_up < check->a_max + profile_a_eps)
        && ruckig_profile_check_for_second_order_velocity_with_timing_ctx(profile, check);
}

bool ruckig_profile_check_for_first_order_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_first_order_check_t* check
) {
    size_t i;
    ruckig_profile_control_signs_t signs;
    ruckig_profile_reached_limits_t limits;
    double v_up;
    if (!profile || !check || profile->t[3] < 0.0) {
        return false;
    }
    signs = check->signs;
    limits = check->limits;
    v_up = check->v_up;

    memset(profile->t_sum, 0, sizeof(profile->t_sum));
    profile->t_sum[3] = profile->t[3];
    profile->t_sum[4] = profile->t[3];
    profile->t_sum[5] = profile->t[3];
    profile->t_sum[6] = profile->t[3];
    if (profile->t_sum[6] > RUCKIG_C_PROFILE_T_MAX) {
        return false;
    }

    memset(profile->j, 0, sizeof(profile->j));
    memset(profile->a, 0, sizeof(profile->a));
    memset(profile->v, 0, sizeof(profile->v));
    profile->a[7] = profile->af;
    profile->v[3] = profile->t[3] > 0.0 ? v_up : 0.0;
    profile->v[7] = profile->vf;
    for (i = 0; i < 7; ++i) {
        profile->p[i + 1] = profile->p[i] + profile->t[i] * (profile->v[i] + profile->t[i] * profile->a[i] / 2.0);
    }

    profile->control_signs = signs;
    profile->limits = limits;
    profile->direction = v_up > 0.0 ? RUCKIG_PROFILE_DIRECTION_UP : RUCKIG_PROFILE_DIRECTION_DOWN;
    return fabs(profile->p[7] - profile->pf) < profile_p_precision;
}

bool ruckig_profile_check_for_first_order_with_timing_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_first_order_check_t* check
) {
    if (check) {
        (void)check->tf;
    }
    return ruckig_profile_check_for_first_order_ctx(profile, check);
}

bool ruckig_profile_check_for_first_order_with_timing_guarded_ctx(
    ruckig_profile_t* profile,
    const ruckig_profile_first_order_check_t* check
) {
    return check
        && (check->v_min - profile_v_eps < check->v_up)
        && (check->v_up < check->v_max + profile_v_eps)
        && ruckig_profile_check_for_first_order_with_timing_ctx(profile, check);
}

static void check_position_extremum(double t_ext, double t_sum, double t, double p, double v, double a, double j, ruckig_bound_t* ext) {
    if (0.0 < t_ext && t_ext < t) {
        double p_ext;
        double a_ext;
        ruckig_integrate(t_ext, p, v, a, j, &p_ext, NULL, &a_ext);
        if (a_ext > 0.0 && p_ext < ext->min) {
            ext->min = p_ext;
            ext->t_min = t_sum + t_ext;
        } else if (a_ext < 0.0 && p_ext > ext->max) {
            ext->max = p_ext;
            ext->t_max = t_sum + t_ext;
        }
    }
}

static void check_step_for_position_extremum(double t_sum, double t, double p, double v, double a, double j, ruckig_bound_t* ext) {
    if (p < ext->min) {
        ext->min = p;
        ext->t_min = t_sum;
    }
    if (p > ext->max) {
        ext->max = p;
        ext->t_max = t_sum;
    }

    if (j != 0.0) {
        const double D = a * a - 2.0 * j * v;
        if (fabs(D) < DBL_EPSILON) {
            check_position_extremum(-a / j, t_sum, t, p, v, a, j, ext);
        } else if (D > 0.0) {
            const double D_sqrt = sqrt(D);
            check_position_extremum((-a - D_sqrt) / j, t_sum, t, p, v, a, j, ext);
            check_position_extremum((-a + D_sqrt) / j, t_sum, t, p, v, a, j, ext);
        }
    }
}

ruckig_bound_t ruckig_profile_get_position_extrema(const ruckig_profile_t* profile) {
    ruckig_bound_t extrema;
    size_t i;
    extrema.min = INFINITY;
    extrema.max = -INFINITY;
    extrema.t_min = 0.0;
    extrema.t_max = 0.0;

    if (!profile) {
        return extrema;
    }

    if (profile->brake.duration > 0.0 && profile->brake.t[0] > 0.0) {
        check_step_for_position_extremum(0.0, profile->brake.t[0], profile->brake.p[0], profile->brake.v[0], profile->brake.a[0], profile->brake.j[0], &extrema);
        if (profile->brake.t[1] > 0.0) {
            check_step_for_position_extremum(profile->brake.t[0], profile->brake.t[1], profile->brake.p[1], profile->brake.v[1], profile->brake.a[1], profile->brake.j[1], &extrema);
        }
    }

    for (i = 0; i < 7; ++i) {
        const double t_current_sum = i > 0 ? profile->t_sum[i - 1] : 0.0;
        check_step_for_position_extremum(t_current_sum + profile->brake.duration, profile->t[i], profile->p[i], profile->v[i], profile->a[i], profile->j[i], &extrema);
    }

    if (profile->pf < extrema.min) {
        extrema.min = profile->pf;
        extrema.t_min = profile->t_sum[6] + profile->brake.duration;
    }
    if (profile->pf > extrema.max) {
        extrema.max = profile->pf;
        extrema.t_max = profile->t_sum[6] + profile->brake.duration;
    }

    return extrema;
}

bool ruckig_profile_get_first_state_at_position(const ruckig_profile_t* profile, double position, double* time, double time_after) {
    double t_cum = 0.0;
    size_t i;
    if (!profile || !time) {
        return false;
    }

    for (i = 0; i < 7; ++i) {
        ruckig_root_set3_t roots;
        size_t k;
        if (profile->t[i] == 0.0) {
            continue;
        }

        if (fabs(profile->p[i] - position) < profile_first_position_precision && t_cum >= time_after) {
            *time = t_cum;
            return true;
        }

        roots = ruckig_solve_cubic(profile->j[i] / 6.0, profile->a[i] / 2.0, profile->v[i], profile->p[i] - position);
        for (k = 0; k < roots.count; ++k) {
            const double root = roots.values[k];
            if (0.0 < root && time_after - t_cum <= root && root <= profile->t[i]) {
                *time = root + t_cum;
                return true;
            }
        }

        t_cum += profile->t[i];
    }

    if ((profile->t[6] > 0.0 || profile->t_sum[6] == 0.0) && fabs(profile->pf - position) < RUCKIG_C_PROFILE_POSITION_MATCH_PRECISION && profile->t_sum[6] >= time_after) {
        *time = profile->t_sum[6];
        return true;
    }

    return false;
}
