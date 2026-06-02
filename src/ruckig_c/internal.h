#ifndef RUCKIG_C_INTERNAL_H
#define RUCKIG_C_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>

#include <ruckig_c/ruckig.h>

#include "ruckig_c/alloc.h"
#include "ruckig_c/block.h"
#include "ruckig_c/profile.h"

struct ruckig_trajectory {
    size_t dofs;
    double duration;
    bool valid;
    ruckig_profile_t* profiles;
    ruckig_block_t* blocks;
    double* independent_min_durations;
    double* cumulative_times;
};

struct ruckig_input {
    size_t dofs;
    ruckig_control_interface_t control_interface;
    ruckig_synchronization_t synchronization;
    ruckig_duration_discretization_t duration_discretization;
    double* current_position;
    double* current_velocity;
    double* current_acceleration;
    double* target_position;
    double* target_velocity;
    double* target_acceleration;
    double* max_velocity;
    double* max_acceleration;
    double* max_jerk;
    bool* enabled;
    bool has_min_velocity;
    double* min_velocity;
    bool has_min_acceleration;
    double* min_acceleration;
    bool has_minimum_duration;
    double minimum_duration;
};

struct ruckig_output {
    size_t dofs;
    struct ruckig_trajectory* trajectory;
    double* new_position;
    double* new_velocity;
    double* new_acceleration;
    double* new_jerk;
    double time;
    size_t new_section;
    bool did_section_change;
    bool new_calculation;
    bool was_calculation_interrupted;
    double calculation_duration;
};

struct ruckig {
    size_t dofs;
    double delta_time;
    bool current_input_initialized;
    struct ruckig_input* current_input;
};

ruckig_result_t ruckig_input_copy_state(const ruckig_input_t* src, ruckig_input_t* dst);
bool ruckig_input_same_dofs(const ruckig_input_t* input, size_t dofs);
bool ruckig_input_equals(const ruckig_input_t* lhs, const ruckig_input_t* rhs);

#endif
