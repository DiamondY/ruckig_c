#include "ruckig_c/position_first.h"

#include <math.h>

bool ruckig_position_first_step2_get_profile(
    ruckig_profile_t* profile,
    double tf,
    double p0,
    double pf,
    double v_max,
    double v_min
) {
    const double pd = pf - p0;
    double vf;

    if (!profile || tf <= 0.0 || !isfinite(tf)) {
        return false;
    }

    vf = pd / tf;

    profile->t[0] = 0.0;
    profile->t[1] = 0.0;
    profile->t[2] = 0.0;
    profile->t[3] = tf;
    profile->t[4] = 0.0;
    profile->t[5] = 0.0;
    profile->t[6] = 0.0;

    return ruckig_profile_check_for_first_order_with_timing_guarded(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, tf, vf, v_max, v_min);
}
