#ifndef RUCKIG_C_POSITION_FIRST_H
#define RUCKIG_C_POSITION_FIRST_H

#include "ruckig_c/block.h"
#include "ruckig_c/profile.h"

#include <stdbool.h>

bool ruckig_position_first_step1_get_profile(
    const ruckig_profile_t* input,
    ruckig_profile_t* output,
    ruckig_block_t* block,
    double* duration,
    double p0,
    double pf,
    double v_max,
    double v_min
);

bool ruckig_position_first_step2_get_profile(
    ruckig_profile_t* profile,
    double tf,
    double p0,
    double pf,
    double v_max,
    double v_min
);

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
);

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
);

bool ruckig_position_third_step1_get_profile(
    const ruckig_profile_t* input,
    ruckig_profile_t* output,
    ruckig_block_t* block,
    double* duration,
    double p0,
    double v0,
    double a0,
    double pf,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
);

bool ruckig_position_third_step2_get_profile(
    ruckig_profile_t* profile,
    double tf,
    double p0,
    double v0,
    double a0,
    double pf,
    double vf,
    double af,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
);

#endif
