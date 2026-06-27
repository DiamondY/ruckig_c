#include "ruckig_c/velocity_second.h"

#include <float.h>
#include <math.h>
#include <string.h>

static void clear_times(ruckig_profile_t* profile) {
    memset(profile->t, 0, sizeof(profile->t));
}

static bool time_acc0(
    ruckig_profile_t* profile,
    double tf,
    double vd,
    double a0,
    double af,
    double a_max,
    double a_min,
    double j_max
) {
    const double ad = af - a0;

    {
        const double radicand = (-ad * ad + 2.0 * j_max * ((a0 + af) * tf - 2.0 * vd)) / (j_max * j_max) + tf * tf;
        if (isfinite(radicand) && radicand >= 0.0) {
            const double h1 = sqrt(radicand);
            clear_times(profile);
            profile->t[0] = ad / (2.0 * j_max) + (tf - h1) / 2.0;
            profile->t[1] = h1;
            profile->t[2] = tf - (profile->t[0] + h1);
            if (ruckig_profile_check_for_velocity_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, tf, j_max, a_max, a_min)) {
                profile->pf = profile->p[7];
                return true;
            }
        }
    }

    {
        const double h1 = -ad + j_max * tf;
        if (fabs(h1) > DBL_EPSILON) {
            clear_times(profile);
            profile->t[0] = -ad * ad / (2.0 * j_max * h1) + (vd - a0 * tf) / h1;
            profile->t[1] = -ad / j_max + tf;
            profile->t[6] = tf - (profile->t[0] + profile->t[1]);
            if (ruckig_profile_check_for_velocity_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, tf, j_max, a_max, a_min)) {
                profile->pf = profile->p[7];
                return true;
            }
        }
    }

    clear_times(profile);
    profile->t[1] = -(af - a0) / j_max + tf;
    profile->t[6] = (af - a0) / j_max;
    if (ruckig_profile_check_for_velocity_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, tf, j_max, a_max, a_min)) {
        profile->pf = profile->p[7];
        return true;
    }

    return false;
}

static bool time_none(
    ruckig_profile_t* profile,
    double tf,
    double vd,
    double a0,
    double af,
    double a_max,
    double a_min,
    double j_max
) {
    const double ad = af - a0;

    if (fabs(a0) < DBL_EPSILON && fabs(af) < DBL_EPSILON && fabs(vd) < DBL_EPSILON) {
        clear_times(profile);
        profile->t[1] = tf;
        if (ruckig_profile_check_for_velocity_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, j_max, a_max, a_min)) {
            profile->pf = profile->p[7];
            return true;
        }
    }

    {
        const double h1 = 2.0 * (af * tf - vd);
        if (fabs(ad) > DBL_EPSILON && fabs(h1) > DBL_EPSILON) {
            const double jf = ad * ad / h1;
            clear_times(profile);
            profile->t[0] = h1 / ad;
            profile->t[1] = tf - profile->t[0];

            if (fabs(jf) < fabs(j_max) + 1e-12
                && ruckig_profile_check_for_velocity_with_timing(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, jf, a_max, a_min)) {
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
    double vd,
    double a0,
    double af,
    double a_max,
    double a_min,
    double j_max
) {
    return time_acc0(profile, tf, vd, a0, af, a_max, a_min, j_max)
        || time_none(profile, tf, vd, a0, af, a_max, a_min, j_max);
}

bool ruckig_velocity_third_step2_get_profile(
    ruckig_profile_t* profile,
    double tf,
    double v0,
    double a0,
    double vf,
    double af,
    double a_max,
    double a_min,
    double j_max
) {
    const double vd = vf - v0;
    if (!profile || tf <= 0.0 || !isfinite(tf)) {
        return false;
    }

    if (vd > 0.0) {
        return check_all(profile, tf, vd, a0, af, a_max, a_min, j_max)
            || check_all(profile, tf, vd, a0, af, a_min, a_max, -j_max);
    }

    return check_all(profile, tf, vd, a0, af, a_min, a_max, -j_max)
        || check_all(profile, tf, vd, a0, af, a_max, a_min, j_max);
}
