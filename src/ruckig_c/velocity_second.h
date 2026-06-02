#ifndef RUCKIG_C_VELOCITY_SECOND_H
#define RUCKIG_C_VELOCITY_SECOND_H

#include "ruckig_c/block.h"
#include "ruckig_c/profile.h"

#include <stdbool.h>

bool ruckig_velocity_second_step1_get_profile(
    const ruckig_profile_t* input,
    ruckig_profile_t* output,
    double* duration,
    double v0,
    double vf,
    double a_max,
    double a_min
);

bool ruckig_velocity_second_step2_get_profile(
    ruckig_profile_t* profile,
    double tf,
    double v0,
    double vf,
    double a_max,
    double a_min
);

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
);

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
);

#endif
