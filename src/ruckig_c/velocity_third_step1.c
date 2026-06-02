#include "ruckig_c/velocity_second.h"

#include <float.h>
#include <math.h>
#include <string.h>

static void clear_times(ruckig_profile_t* profile) {
    memset(profile->t, 0, sizeof(profile->t));
}

static bool candidate_time_acc0(
    ruckig_profile_t* profile,
    double vd,
    double a0,
    double af,
    double a_max,
    double a_min,
    double j_max
) {
    clear_times(profile);
    profile->t[0] = (-a0 + a_max) / j_max;
    profile->t[1] = (a0 * a0 + af * af) / (2.0 * a_max * j_max) - a_max / j_max + vd / a_max;
    profile->t[2] = (-af + a_max) / j_max;

    return ruckig_profile_check_for_velocity(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, j_max, a_max, a_min);
}

static bool candidate_time_none_1(
    ruckig_profile_t* profile,
    double h1,
    double a0,
    double af,
    double a_max,
    double a_min,
    double j_max
) {
    clear_times(profile);
    profile->t[0] = -(a0 + h1) / j_max;
    profile->t[2] = -(af + h1) / j_max;
    return ruckig_profile_check_for_velocity(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, j_max, a_max, a_min);
}

static bool candidate_time_none_2(
    ruckig_profile_t* profile,
    double h1,
    double a0,
    double af,
    double a_max,
    double a_min,
    double j_max
) {
    clear_times(profile);
    profile->t[0] = (-a0 + h1) / j_max;
    profile->t[2] = (-af + h1) / j_max;
    return ruckig_profile_check_for_velocity(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, j_max, a_max, a_min);
}

static size_t append_time_none(
    ruckig_profile_t* valid,
    size_t count,
    const ruckig_profile_t* input,
    double vd,
    double a0,
    double af,
    double a_max,
    double a_min,
    double j_max,
    bool return_after_found
) {
    double h1 = (a0 * a0 + af * af) / 2.0 + j_max * vd;
    if (h1 >= 0.0) {
        ruckig_profile_t candidate;
        h1 = sqrt(h1);

        ruckig_profile_copy_boundary(&candidate, input);
        if (candidate_time_none_1(&candidate, h1, a0, af, a_max, a_min, j_max)) {
            valid[count++] = candidate;
            if (return_after_found) {
                return count;
            }
        }

        ruckig_profile_copy_boundary(&candidate, input);
        if (candidate_time_none_2(&candidate, h1, a0, af, a_max, a_min, j_max)) {
            valid[count++] = candidate;
        }
    }
    return count;
}

static size_t append_time_acc0(
    ruckig_profile_t* valid,
    size_t count,
    const ruckig_profile_t* input,
    double vd,
    double a0,
    double af,
    double a_max,
    double a_min,
    double j_max
) {
    ruckig_profile_t candidate;
    ruckig_profile_copy_boundary(&candidate, input);
    if (candidate_time_acc0(&candidate, vd, a0, af, a_max, a_min, j_max)) {
        valid[count++] = candidate;
    }
    return count;
}

static bool time_all_single_step(
    ruckig_profile_t* profile,
    double vd,
    double a0,
    double af,
    double a_max,
    double a_min
) {
    clear_times(profile);
    if (fabs(af - a0) > DBL_EPSILON) {
        return false;
    }
    if (fabs(a0) > DBL_EPSILON) {
        profile->t[3] = vd / a0;
        return ruckig_profile_check_for_velocity(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, 0.0, a_max, a_min);
    }
    if (fabs(vd) < DBL_EPSILON) {
        return ruckig_profile_check_for_velocity(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, 0.0, a_max, a_min);
    }
    return false;
}

bool ruckig_velocity_third_step1_get_profile(
    const ruckig_profile_t* input,
    ruckig_profile_t* output,
    ruckig_block_t* block,
    double* duration,
    double v0,
    double a0,
    double vf,
    double af,
    double a_max,
    double a_min,
    double j_max
) {
    ruckig_profile_t valid[4];
    size_t count = 0;
    const double vd = vf - v0;

    if (!input || !output || !block || !duration) {
        return false;
    }

    ruckig_block_init(block);
    if (j_max == 0.0) {
        ruckig_profile_copy_boundary(output, input);
        if (time_all_single_step(output, vd, a0, af, a_max, a_min)) {
            block->p_min = *output;
            block->t_min = output->t_sum[6] + output->brake.duration + output->accel.duration;
            block->valid = true;
            if (fabs(a0) > DBL_EPSILON) {
                block->a.valid = true;
                block->a.left = block->t_min;
                block->a.right = INFINITY;
                block->a.profile = *output;
            }
            *duration = output->t_sum[6] + output->brake.duration + output->accel.duration;
            return true;
        }
        return false;
    }

    if (fabs(af) < DBL_EPSILON) {
        const bool positive = vd >= 0.0;
        const double aMax = positive ? a_max : a_min;
        const double aMin = positive ? a_min : a_max;
        const double jMax = positive ? j_max : -j_max;

        count = append_time_none(valid, count, input, vd, a0, af, aMax, aMin, jMax, true);
        if (count == 0) {
            count = append_time_acc0(valid, count, input, vd, a0, af, aMax, aMin, jMax);
        }
        if (count == 0) {
            count = append_time_none(valid, count, input, vd, a0, af, aMin, aMax, -jMax, true);
        }
        if (count == 0) {
            count = append_time_acc0(valid, count, input, vd, a0, af, aMin, aMax, -jMax);
        }
    } else {
        count = append_time_none(valid, count, input, vd, a0, af, a_max, a_min, j_max, false);
        count = append_time_none(valid, count, input, vd, a0, af, a_min, a_max, -j_max, false);
        count = append_time_acc0(valid, count, input, vd, a0, af, a_max, a_min, j_max);
        count = append_time_acc0(valid, count, input, vd, a0, af, a_min, a_max, -j_max);
    }

    if (count == 0) {
        return false;
    }
    if (!ruckig_block_calculate(block, valid, count)) {
        return false;
    }
    *output = block->p_min;
    *duration = output->t_sum[6] + output->brake.duration + output->accel.duration;
    return true;
}
