#include "ruckig_c/block.h"

#include "ruckig_c/precision.h"

#include <float.h>
#include <math.h>
#include <string.h>

static double profile_duration(const ruckig_profile_t* profile) {
    return profile->t_sum[6] + profile->brake.duration + profile->accel.duration;
}

static void remove_profile(ruckig_profile_t* valid_profiles, size_t* valid_profile_count, size_t index) {
    size_t i;
    for (i = index; i + 1 < *valid_profile_count; ++i) {
        valid_profiles[i] = valid_profiles[i + 1];
    }
    *valid_profile_count -= 1;
}

static void block_set_min_profile(ruckig_block_t* block, const ruckig_profile_t* profile) {
    block->p_min = *profile;
    block->t_min = profile_duration(profile);
    block->a.valid = false;
    block->b.valid = false;
    block->valid = true;
}

static ruckig_block_interval_t make_interval(const ruckig_profile_t* profile_left, const ruckig_profile_t* profile_right) {
    ruckig_block_interval_t interval;
    const double left_duration = profile_duration(profile_left);
    const double right_duration = profile_duration(profile_right);

    memset(&interval, 0, sizeof(interval));
    interval.valid = true;
    if (left_duration < right_duration) {
        interval.left = left_duration;
        interval.right = right_duration;
        interval.profile = *profile_right;
    } else {
        interval.left = right_duration;
        interval.right = left_duration;
        interval.profile = *profile_left;
    }
    return interval;
}

void ruckig_block_init(ruckig_block_t* block) {
    if (!block) {
        return;
    }
    memset(block, 0, sizeof(*block));
    ruckig_profile_init(&block->p_min);
}

bool ruckig_block_calculate(ruckig_block_t* block, ruckig_profile_t* valid_profiles, size_t valid_profile_count) {
    size_t idx_min = 0;
    size_t i;

    if (!block || !valid_profiles || valid_profile_count == 0) {
        return false;
    }

    if (valid_profile_count == 1) {
        block_set_min_profile(block, &valid_profiles[0]);
        return true;
    } else if (valid_profile_count == 2) {
        /* Preserve oracle candidate ordering: equal-duration alternatives collapse to the first profile. */
        if (fabs(profile_duration(&valid_profiles[0]) - profile_duration(&valid_profiles[1])) < RUCKIG_C_BLOCK_DURATION_TIE_EPS_2 * DBL_EPSILON) {
            block_set_min_profile(block, &valid_profiles[0]);
            return true;
        }

        idx_min = profile_duration(&valid_profiles[0]) < profile_duration(&valid_profiles[1]) ? 0 : 1;
        block_set_min_profile(block, &valid_profiles[idx_min]);
        block->a = make_interval(&valid_profiles[idx_min], &valid_profiles[(idx_min + 1) % 2]);
        return true;

    } else if (valid_profile_count == 4) {
        /* Ruckig Step1 may emit a mirrored pair around a blocked interval; remove only the oracle-matched duplicate. */
        if (fabs(profile_duration(&valid_profiles[0]) - profile_duration(&valid_profiles[1])) < RUCKIG_C_BLOCK_DURATION_TIE_EPS_4_NEAR * DBL_EPSILON
            && valid_profiles[0].direction != valid_profiles[1].direction) {
            remove_profile(valid_profiles, &valid_profile_count, 1);
        } else if (fabs(profile_duration(&valid_profiles[2]) - profile_duration(&valid_profiles[3])) < RUCKIG_C_BLOCK_DURATION_TIE_EPS_4_FAR * DBL_EPSILON
            && valid_profiles[2].direction != valid_profiles[3].direction) {
            remove_profile(valid_profiles, &valid_profile_count, 3);
        } else if (fabs(profile_duration(&valid_profiles[0]) - profile_duration(&valid_profiles[3])) < RUCKIG_C_BLOCK_DURATION_TIE_EPS_4_FAR * DBL_EPSILON
            && valid_profiles[0].direction != valid_profiles[3].direction) {
            remove_profile(valid_profiles, &valid_profile_count, 3);
        } else {
            return false;
        }
    } else if ((valid_profile_count % 2) == 0) {
        return false;
    }

    for (i = 1; i < valid_profile_count; ++i) {
        if (profile_duration(&valid_profiles[i]) < profile_duration(&valid_profiles[idx_min])) {
            idx_min = i;
        }
    }

    block_set_min_profile(block, &valid_profiles[idx_min]);

    if (valid_profile_count == 3) {
        const size_t idx_else_1 = (idx_min + 1) % 3;
        const size_t idx_else_2 = (idx_min + 2) % 3;
        block->a = make_interval(&valid_profiles[idx_else_1], &valid_profiles[idx_else_2]);
        return true;
    } else if (valid_profile_count == 5) {
        const size_t idx_else_1 = (idx_min + 1) % 5;
        const size_t idx_else_2 = (idx_min + 2) % 5;
        const size_t idx_else_3 = (idx_min + 3) % 5;
        const size_t idx_else_4 = (idx_min + 4) % 5;

        if (valid_profiles[idx_else_1].direction == valid_profiles[idx_else_2].direction) {
            block->a = make_interval(&valid_profiles[idx_else_1], &valid_profiles[idx_else_2]);
            block->b = make_interval(&valid_profiles[idx_else_3], &valid_profiles[idx_else_4]);
        } else {
            block->a = make_interval(&valid_profiles[idx_else_1], &valid_profiles[idx_else_4]);
            block->b = make_interval(&valid_profiles[idx_else_2], &valid_profiles[idx_else_3]);
        }
        return true;
    }

    return false;
}

bool ruckig_block_is_blocked(const ruckig_block_t* block, double t) {
    if (!block || !block->valid) {
        return true;
    }
    return (t < block->t_min)
        || (block->a.valid && block->a.left < t && t < block->a.right)
        || (block->b.valid && block->b.left < t && t < block->b.right);
}

const ruckig_profile_t* ruckig_block_get_profile(const ruckig_block_t* block, double t) {
    if (!block || !block->valid) {
        return NULL;
    }
    if (block->b.valid && t >= block->b.right) {
        return &block->b.profile;
    }
    if (block->a.valid && t >= block->a.right) {
        return &block->a.profile;
    }
    return &block->p_min;
}
