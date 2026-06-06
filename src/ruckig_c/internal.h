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
    size_t max_number_of_waypoints;
    size_t section_capacity;
    size_t section_count;
    double duration;
    bool valid;
    ruckig_profile_t* profiles;
    ruckig_block_t* blocks;
    double* independent_min_durations;
    double* cumulative_times;
};

struct ruckig_input {
    size_t dofs;
    size_t max_number_of_waypoints;
    size_t waypoint_count;
    ruckig_control_interface_t control_interface;
    ruckig_synchronization_t synchronization;
    bool has_per_dof_control_interface;
    ruckig_control_interface_t* per_dof_control_interface;
    bool has_per_dof_synchronization;
    ruckig_synchronization_t* per_dof_synchronization;
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
    double* max_position;
    double* min_position;
    bool* enabled;
    bool has_min_velocity;
    double* min_velocity;
    bool has_min_acceleration;
    double* min_acceleration;
    bool has_minimum_duration;
    double minimum_duration;
    double* intermediate_positions;
    bool has_per_section_max_velocity;
    double* per_section_max_velocity;
    bool has_per_section_min_velocity;
    double* per_section_min_velocity;
    bool has_per_section_max_acceleration;
    double* per_section_max_acceleration;
    bool has_per_section_min_acceleration;
    double* per_section_min_acceleration;
    bool has_per_section_max_jerk;
    double* per_section_max_jerk;
    bool has_per_section_max_position;
    double* per_section_max_position;
    bool has_per_section_min_position;
    double* per_section_min_position;
    bool has_per_section_minimum_duration;
    double* per_section_minimum_duration;
    bool has_interrupt_calculation_duration;
    double interrupt_calculation_duration;
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
    size_t max_number_of_waypoints;
    double delta_time;
    bool current_input_initialized;
    struct ruckig_input* current_input;
    struct ruckig_input* waypoint_section_input;
    struct ruckig_trajectory* waypoint_section_trajectory;
    double* waypoint_candidate_velocity;
    double* waypoint_candidate_acceleration;
    double* waypoint_best_velocity;
    double* waypoint_best_acceleration;
    double* waypoint_baseline_velocity;
    double* waypoint_baseline_acceleration;
    double waypoint_last_baseline_duration;
    double waypoint_last_best_duration;
    double waypoint_last_best_lower_bound;
    size_t waypoint_last_candidate_evaluations;
    bool waypoint_last_improved_baseline;
};

struct ruckig_target_state {
    size_t dofs;
    double* position;
    double* velocity;
    double* acceleration;
};

struct ruckig_target_state_sequence {
    size_t dofs;
    size_t capacity;
    size_t count;
    double* position;
    double* velocity;
    double* acceleration;
};

struct ruckig_tracking_output_sequence {
    size_t dofs;
    size_t capacity;
    size_t count;
    double* new_position;
    double* new_velocity;
    double* new_acceleration;
    double* new_jerk;
    double* time;
    size_t* section;
    ruckig_result_t* result;
};

struct ruckig_tracking {
    size_t dofs;
    double delta_time;
    ruckig_tracking_mode_t mode;
    double reactiveness;
    size_t look_ahead_cycles;
    struct ruckig* otg;
    struct ruckig_input* work_input;
    struct ruckig_output* work_output;
};

ruckig_result_t ruckig_input_copy_state(const ruckig_input_t* src, ruckig_input_t* dst);
bool ruckig_input_same_dofs(const ruckig_input_t* input, size_t dofs);
bool ruckig_input_equals(const ruckig_input_t* lhs, const ruckig_input_t* rhs);
ruckig_result_t ruckig_calculate_target(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
);
ruckig_result_t ruckig_calculate_waypoints(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
);

#endif
