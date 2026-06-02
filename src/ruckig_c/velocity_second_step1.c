#include "ruckig_c/velocity_second.h"

#include <math.h>
#include <string.h>

bool ruckig_velocity_second_step1_get_profile(
    const ruckig_profile_t* input,
    ruckig_profile_t* output,
    double* duration,
    double v0,
    double vf,
    double a_max,
    double a_min
) {
    const double vd = vf - v0;
    const double af = vd > 0.0 ? a_max : a_min;

    if (!input || !output || !duration || af == 0.0 || !isfinite(af)) {
        return false;
    }

    ruckig_profile_copy_boundary(output, input);
    memset(output->t, 0, sizeof(output->t));
    output->t[1] = vd / af;

    if (ruckig_profile_check_for_second_order_velocity(output, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, af)) {
        *duration = output->t_sum[6] + output->brake.duration + output->accel.duration;
        return true;
    }

    return false;
}
