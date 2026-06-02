#include "ruckig_c/position_first.h"

#include <math.h>

bool ruckig_position_first_step1_get_profile(
    const ruckig_profile_t* input,
    ruckig_profile_t* output,
    ruckig_block_t* block,
    double* duration,
    double p0,
    double pf,
    double v_max,
    double v_min
) {
    const double pd = pf - p0;
    const double vf = pd > 0.0 ? v_max : v_min;

    if (!input || !output || !block || !duration || vf == 0.0 || !isfinite(vf)) {
        return false;
    }

    ruckig_block_init(block);
    ruckig_profile_copy_boundary(output, input);
    output->t[0] = 0.0;
    output->t[1] = 0.0;
    output->t[2] = 0.0;
    output->t[3] = pd / vf;
    output->t[4] = 0.0;
    output->t[5] = 0.0;
    output->t[6] = 0.0;

    if (ruckig_profile_check_for_first_order(output, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_VEL, vf)) {
        block->p_min = *output;
        block->t_min = output->t_sum[6] + output->brake.duration + output->accel.duration;
        block->valid = true;
        *duration = output->t_sum[6] + output->brake.duration + output->accel.duration;
        return true;
    }

    return false;
}
