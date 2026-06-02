#include "ruckig_c/position_first.h"

#include <float.h>
#include <math.h>
#include <string.h>

static void clear_times(ruckig_profile_t* profile) {
    memset(profile->t, 0, sizeof(profile->t));
}

static bool time_acc0(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double vf,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    const double vd = vf - v0;

    {
        const double radicand = (2.0 * a_max * (pd - tf * vf) - 2.0 * a_min * (pd - tf * v0) + vd * vd) / (a_max * a_min) + tf * tf;
        if (radicand >= 0.0) {
            const double h1 = sqrt(radicand);
            clear_times(profile);
            profile->t[0] = (a_max * vd - a_max * a_min * (tf - h1)) / (a_max * (a_max - a_min));
            profile->t[1] = h1;
            profile->t[2] = tf - (profile->t[0] + h1);
            if (ruckig_profile_check_for_second_order_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, tf, a_max, a_min, v_max, v_min)) {
                profile->pf = profile->p[7];
                return true;
            }
        }
    }

    {
        const double h1 = -vd + a_max * tf;
        if (fabs(h1) > DBL_EPSILON) {
            clear_times(profile);
            profile->t[0] = -vd * vd / (2.0 * a_max * h1) + (pd - v0 * tf) / h1;
            profile->t[1] = -vd / a_max + tf;
            profile->t[6] = tf - (profile->t[0] + profile->t[1]);
            if (ruckig_profile_check_for_second_order_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, tf, a_max, a_min, v_max, v_min)) {
                profile->pf = profile->p[7];
                return true;
            }
        }
    }

    clear_times(profile);
    profile->t[1] = -vd / a_max + tf;
    profile->t[6] = vd / a_max;
    if (ruckig_profile_check_for_second_order_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, tf, a_max, a_min, v_max, v_min)) {
        profile->pf = profile->p[7];
        return true;
    }

    return false;
}

static bool time_none(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double vf,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    const double vd = vf - v0;
    if (fabs(v0) < DBL_EPSILON && fabs(vf) < DBL_EPSILON && fabs(pd) < DBL_EPSILON) {
        clear_times(profile);
        profile->t[1] = tf;
        if (ruckig_profile_check_for_second_order_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, a_max, a_min, v_max, v_min)) {
            profile->pf = profile->p[7];
            return true;
        }
    }

    {
        const double h1 = 2.0 * (vf * tf - pd);
        if (fabs(vd) > DBL_EPSILON && fabs(h1) > DBL_EPSILON) {
            const double af = vd * vd / h1;
            clear_times(profile);
            profile->t[0] = h1 / vd;
            profile->t[1] = tf - profile->t[0];

            if ((a_min - 1e-12 < af) && (af < a_max + 1e-12)
                && ruckig_profile_check_for_second_order_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, af, -af, v_max, v_min)) {
                profile->pf = profile->p[7];
                return true;
            }
        }
    }

    return false;
}

static bool check_all(
    ruckig_profile_t* profile,
    double tf,
    double pd,
    double v0,
    double vf,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    return time_acc0(profile, tf, pd, v0, vf, v_max, v_min, a_max, a_min)
        || time_none(profile, tf, pd, v0, vf, v_max, v_min, a_max, a_min);
}

bool ruckig_position_second_step2_get_profile(
    ruckig_profile_t* profile,
    double tf,
    double p0,
    double v0,
    double pf,
    double vf,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    const double pd = pf - p0;
    if (!profile || tf < 0.0 || !isfinite(tf)) {
        return false;
    }

    if (pd > 0.0) {
        return check_all(profile, tf, pd, v0, vf, v_max, v_min, a_max, a_min)
            || check_all(profile, tf, pd, v0, vf, v_min, v_max, a_min, a_max);
    }

    return check_all(profile, tf, pd, v0, vf, v_min, v_max, a_min, a_max)
        || check_all(profile, tf, pd, v0, vf, v_max, v_min, a_max, a_min);
}
