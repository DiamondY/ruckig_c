#include "ruckig_c/position_first.h"

#include <float.h>
#include <math.h>
#include <string.h>

enum {
    RUCKIG_POSITION_SECOND_STEP1_MAX_CANDIDATES = 8
};

static void clear_times(ruckig_profile_t* profile) {
    memset(profile->t, 0, sizeof(profile->t));
}

static bool candidate_time_acc0(
    ruckig_profile_t* profile,
    double v0,
    double vf,
    double pd,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    clear_times(profile);
    profile->t[0] = (-v0 + v_max) / a_max;
    profile->t[1] = (a_min * v0 * v0 - a_max * vf * vf) / (2.0 * a_max * a_min * v_max)
        + v_max * (a_max - a_min) / (2.0 * a_max * a_min)
        + pd / v_max;
    profile->t[2] = (vf - v_max) / a_min;

    return ruckig_profile_check_for_second_order(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, a_max, a_min, v_max, v_min);
}

static bool candidate_time_none_1(
    ruckig_profile_t* profile,
    double v0,
    double vf,
    double h1,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    clear_times(profile);
    profile->t[0] = -(v0 + h1) / a_max;
    profile->t[2] = (vf + h1) / a_min;
    return ruckig_profile_check_for_second_order(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, a_max, a_min, v_max, v_min);
}

static bool candidate_time_none_2(
    ruckig_profile_t* profile,
    double v0,
    double vf,
    double h1,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    clear_times(profile);
    profile->t[0] = (-v0 + h1) / a_max;
    profile->t[2] = (vf - h1) / a_min;
    return ruckig_profile_check_for_second_order(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, a_max, a_min, v_max, v_min);
}

static size_t append_time_none(
    ruckig_profile_t* valid,
    size_t count,
    size_t capacity,
    const ruckig_profile_t* input,
    double v0,
    double vf,
    double pd,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    bool return_after_found
) {
    double h1 = (a_max * vf * vf - a_min * v0 * v0 - 2.0 * a_max * a_min * pd) / (a_max - a_min);
    if (h1 >= 0.0) {
        ruckig_profile_t candidate;
        h1 = sqrt(h1);

        ruckig_profile_copy_boundary(&candidate, input);
        if (candidate_time_none_1(&candidate, v0, vf, h1, v_max, v_min, a_max, a_min)) {
            if (count < capacity) {
                valid[count++] = candidate;
            }
            if (return_after_found) {
                return count;
            }
        }

        ruckig_profile_copy_boundary(&candidate, input);
        if (candidate_time_none_2(&candidate, v0, vf, h1, v_max, v_min, a_max, a_min)) {
            if (count < capacity) {
                valid[count++] = candidate;
            }
        }
    }
    return count;
}

static size_t append_time_acc0(
    ruckig_profile_t* valid,
    size_t count,
    size_t capacity,
    const ruckig_profile_t* input,
    double v0,
    double vf,
    double pd,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    ruckig_profile_t candidate;
    ruckig_profile_copy_boundary(&candidate, input);
    if (candidate_time_acc0(&candidate, v0, vf, pd, v_max, v_min, a_max, a_min)) {
        if (count < capacity) {
            valid[count++] = candidate;
        }
    }
    return count;
}

static bool time_all_single_step(
    ruckig_profile_t* profile,
    double v0,
    double vf,
    double pd,
    double v_max,
    double v_min
) {
    clear_times(profile);
    if (fabs(vf - v0) > DBL_EPSILON) {
        return false;
    }
    if (fabs(v0) > DBL_EPSILON) {
        profile->t[3] = pd / v0;
        return ruckig_profile_check_for_second_order(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, 0.0, 0.0, v_max, v_min);
    }
    if (fabs(pd) < DBL_EPSILON) {
        return ruckig_profile_check_for_second_order(profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, 0.0, 0.0, v_max, v_min);
    }
    return false;
}

bool ruckig_position_second_step1_get_profile(
    const ruckig_profile_t* input,
    ruckig_profile_t* output,
    ruckig_block_t* block,
    double* duration,
    double p0,
    double v0,
    double pf,
    double vf,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    ruckig_profile_t valid[RUCKIG_POSITION_SECOND_STEP1_MAX_CANDIDATES];
    size_t count = 0;
    const double pd = pf - p0;

    if (!input || !output || !block || !duration) {
        return false;
    }

    ruckig_block_init(block);
    if (v_max == 0.0 && v_min == 0.0) {
        ruckig_profile_copy_boundary(output, input);
        if (time_all_single_step(output, v0, vf, pd, v_max, v_min)) {
            block->p_min = *output;
            block->t_min = output->t_sum[6] + output->brake.duration + output->accel.duration;
            block->valid = true;
            if (fabs(v0) > DBL_EPSILON) {
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

    if (fabs(vf) < DBL_EPSILON) {
        const bool positive = pd >= 0.0;
        const double vMax = positive ? v_max : v_min;
        const double vMin = positive ? v_min : v_max;
        const double aMax = positive ? a_max : a_min;
        const double aMin = positive ? a_min : a_max;
        count = append_time_none(valid, count, RUCKIG_POSITION_SECOND_STEP1_MAX_CANDIDATES, input, v0, vf, pd, vMax, vMin, aMax, aMin, true);
        if (count == 0) {
            count = append_time_acc0(valid, count, RUCKIG_POSITION_SECOND_STEP1_MAX_CANDIDATES, input, v0, vf, pd, vMax, vMin, aMax, aMin);
        }
        if (count == 0) {
            count = append_time_none(valid, count, RUCKIG_POSITION_SECOND_STEP1_MAX_CANDIDATES, input, v0, vf, pd, vMin, vMax, aMin, aMax, true);
        }
        if (count == 0) {
            count = append_time_acc0(valid, count, RUCKIG_POSITION_SECOND_STEP1_MAX_CANDIDATES, input, v0, vf, pd, vMin, vMax, aMin, aMax);
        }
    } else {
        count = append_time_none(valid, count, RUCKIG_POSITION_SECOND_STEP1_MAX_CANDIDATES, input, v0, vf, pd, v_max, v_min, a_max, a_min, false);
        count = append_time_none(valid, count, RUCKIG_POSITION_SECOND_STEP1_MAX_CANDIDATES, input, v0, vf, pd, v_min, v_max, a_min, a_max, false);
        count = append_time_acc0(valid, count, RUCKIG_POSITION_SECOND_STEP1_MAX_CANDIDATES, input, v0, vf, pd, v_max, v_min, a_max, a_min);
        count = append_time_acc0(valid, count, RUCKIG_POSITION_SECOND_STEP1_MAX_CANDIDATES, input, v0, vf, pd, v_min, v_max, a_min, a_max);
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
