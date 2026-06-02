#ifndef RUCKIG_C_BLOCK_H
#define RUCKIG_C_BLOCK_H

#include "ruckig_c/profile.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct ruckig_block_interval {
    double left;
    double right;
    ruckig_profile_t profile;
    bool valid;
} ruckig_block_interval_t;

typedef struct ruckig_block {
    ruckig_profile_t p_min;
    double t_min;
    ruckig_block_interval_t a;
    ruckig_block_interval_t b;
    bool valid;
} ruckig_block_t;

void ruckig_block_init(ruckig_block_t* block);
bool ruckig_block_calculate(ruckig_block_t* block, ruckig_profile_t* valid_profiles, size_t valid_profile_count);
bool ruckig_block_is_blocked(const ruckig_block_t* block, double t);
const ruckig_profile_t* ruckig_block_get_profile(const ruckig_block_t* block, double t);

#endif
