#include "ruckig_c/velocity_second.h"

#include <math.h>
#include <string.h>

bool ruckig_velocity_second_step2_get_profile(
    ruckig_profile_t* profile,
    double tf,
    double v0,
    double vf,
    double a_max,
    double a_min
) {
    const double vd = vf - v0;
    double af;

    if (!profile || tf <= 0.0 || !isfinite(tf)) {
        return false;
    }

    af = vd / tf;
    memset(profile->t, 0, sizeof(profile->t));
    profile->t[1] = tf;

    if (ruckig_profile_check_for_second_order_velocity_with_timing_guarded(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, af, a_max, a_min)) {
        profile->pf = profile->p[7];
        return true;
    }

    return false;
}
