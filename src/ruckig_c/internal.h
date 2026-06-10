#ifndef RUCKIG_C_INTERNAL_H
#define RUCKIG_C_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>

#include <ruckig_c/ruckig.h>

#include "ruckig_c/alloc.h"
#include "ruckig_c/block.h"
#include "ruckig_c/profile.h"

#define RUCKIG_TRACKING_AUDIT_FAMILY_COUNT 6u
#define RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY 64u
#define RUCKIG_WAYPOINT_BRANCH_ITERATION_BUDGET 256u

typedef struct ruckig_waypoint_branch {
    size_t index;
    bool acceleration;
    double delta;
    double lower_bound;
} ruckig_waypoint_branch_t;

typedef enum ruckig_waypoint_resume_phase {
    RUCKIG_WAYPOINT_RESUME_PHASE_IDLE = 0,
    RUCKIG_WAYPOINT_RESUME_PHASE_BASELINE,
    RUCKIG_WAYPOINT_RESUME_PHASE_FINITE_DIFFERENCE_035,
    RUCKIG_WAYPOINT_RESUME_PHASE_FINITE_DIFFERENCE_070,
    RUCKIG_WAYPOINT_RESUME_PHASE_REFINE_INIT,
    RUCKIG_WAYPOINT_RESUME_PHASE_REFINE,
    RUCKIG_WAYPOINT_RESUME_PHASE_BRANCH_INIT,
    RUCKIG_WAYPOINT_RESUME_PHASE_BRANCH,
    RUCKIG_WAYPOINT_RESUME_PHASE_COMPLETE
} ruckig_waypoint_resume_phase_t;

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
    struct ruckig_trajectory* waypoint_resume_trajectory;
    double* waypoint_candidate_velocity;
    double* waypoint_candidate_acceleration;
    double* waypoint_best_velocity;
    double* waypoint_best_acceleration;
    double* waypoint_baseline_velocity;
    double* waypoint_baseline_acceleration;
    struct ruckig_input* waypoint_resume_identity_input;
    ruckig_waypoint_branch_t* waypoint_resume_branch_queue;
    bool waypoint_resume_active;
    bool waypoint_resume_complete;
    bool waypoint_resume_found;
    bool waypoint_resume_initial_calculation;
    bool waypoint_resume_has_published_candidate;
    ruckig_waypoint_resume_phase_t waypoint_resume_phase;
    double waypoint_resume_best_duration;
    double waypoint_resume_baseline_duration;
    double waypoint_resume_published_duration;
    size_t waypoint_resume_refine_pass;
    size_t waypoint_resume_refine_waypoint;
    size_t waypoint_resume_refine_dof;
    size_t waypoint_resume_refine_component;
    size_t waypoint_resume_refine_attempt;
    double waypoint_resume_refine_original;
    bool waypoint_resume_refine_original_valid;
    bool waypoint_resume_refine_improved;
    double waypoint_resume_branch_scale;
    size_t waypoint_resume_branch_iteration;
    size_t waypoint_resume_branch_count;
    size_t waypoint_resume_branch_index;
    bool waypoint_resume_branch_queue_valid;
    bool waypoint_resume_branch_improved_any;
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
    size_t max_optimized_candidates;
    ruckig_tracking_optimized_strategy_t optimized_strategy;
    ruckig_tracking_calculation_status_t last_calculation_status;
    size_t last_candidate_count;
    ruckig_tracking_diagnostics_t diagnostics;
    size_t audit_family_attempted[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t audit_family_valid[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t audit_family_strict_improved[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t audit_family_near_tie_accepted[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t audit_family_selected[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t audit_last_candidate_family;
    size_t audit_best_candidate_family;
    bool audit_best_candidate_near_tie;
    size_t audit_strict_improved_count;
    size_t audit_near_tie_accepted_count;
    double* optimized_candidate_position;
    double* optimized_candidate_velocity;
    double* optimized_candidate_acceleration;
    double* optimized_candidate_jerk;
    double* optimized_best_position;
    double* optimized_best_velocity;
    double* optimized_best_acceleration;
    struct ruckig* otg;
    struct ruckig_input* work_input;
    struct ruckig_output* work_output;
};

ruckig_result_t ruckig_input_copy_state(const ruckig_input_t* src, ruckig_input_t* dst);
bool ruckig_input_same_dofs(const ruckig_input_t* input, size_t dofs);
bool ruckig_input_equals(const ruckig_input_t* lhs, const ruckig_input_t* rhs);
bool ruckig_input_equals_ignoring_interrupt(const ruckig_input_t* lhs, const ruckig_input_t* rhs);
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
ruckig_result_t ruckig_calculate_waypoints_interruptible(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    bool* was_interrupted
);
void ruckig_waypoint_resume_clear(ruckig_t* otg);
bool ruckig_waypoint_resume_can_continue(const ruckig_t* otg, const ruckig_input_t* input);
ruckig_result_t ruckig_waypoint_resume_continue(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    double incumbent_remaining_duration,
    bool* was_interrupted,
    bool* published
);

#endif
