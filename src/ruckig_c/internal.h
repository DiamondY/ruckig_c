#ifndef RUCKIG_C_INTERNAL_H
#define RUCKIG_C_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef RUCKIG_C_ENABLE_INTERNAL_ASSERTS
#include <assert.h>
#include <stdlib.h>
#endif

#include <ruckig_c/ruckig.h>

#include "ruckig_c/alloc.h"
#include "ruckig_c/block.h"
#include "ruckig_c/precision.h"
#include "ruckig_c/profile.h"

#define RUCKIG_TRACKING_AUDIT_FAMILY_COUNT 6u
#define RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY 64u
#define RUCKIG_WAYPOINT_BRANCH_ITERATION_BUDGET 256u
#ifdef RUCKIG_C_ENABLE_INTERNAL_ASSERTS
#define RUCKIG_C_INTERNAL_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            assert(expr); \
            abort(); \
        } \
    } while (0)
#else
#define RUCKIG_C_INTERNAL_ASSERT(expr) ((void)sizeof(expr))
#endif

static inline bool ruckig_checked_add_size(size_t lhs, size_t rhs, size_t* out) {
    if (!out || lhs > SIZE_MAX - rhs) {
        return false;
    }
    *out = lhs + rhs;
    return true;
}

static inline bool ruckig_checked_mul_size(size_t lhs, size_t rhs, size_t* out) {
    if (!out || (lhs != 0u && rhs > SIZE_MAX / lhs)) {
        return false;
    }
    *out = lhs * rhs;
    return true;
}

static inline bool ruckig_checked_waypoint_counts(
    size_t dofs,
    size_t max_waypoints,
    size_t* sections,
    size_t* section_values,
    size_t* waypoint_values
) {
    size_t local_sections = 0;
    size_t local_section_values = 0;
    size_t local_waypoint_values = 0;
    if (dofs == 0u
        || !ruckig_checked_add_size(max_waypoints, 1u, &local_sections)
        || !ruckig_checked_mul_size(local_sections, dofs, &local_section_values)
        || !ruckig_checked_mul_size(max_waypoints, dofs, &local_waypoint_values)) {
        return false;
    }
    if (sections) {
        *sections = local_sections;
    }
    if (section_values) {
        *section_values = local_section_values;
    }
    if (waypoint_values) {
        *waypoint_values = local_waypoint_values;
    }
    return true;
}

static inline size_t ruckig_diagnostics_stable_prefix_size(void) {
    return offsetof(ruckig_diagnostics_t, reserved_size);
}

static inline size_t ruckig_diagnostics_min_size(size_t lhs, size_t rhs) {
    return lhs < rhs ? lhs : rhs;
}

static inline ruckig_result_t ruckig_diagnostics_validate_or_null(const ruckig_diagnostics_t* diagnostics) {
    if (!diagnostics) {
        return RUCKIG_WORKING;
    }
    return diagnostics->struct_size >= ruckig_diagnostics_stable_prefix_size()
        ? RUCKIG_WORKING
        : RUCKIG_ERROR_INVALID_INPUT;
}

static inline void ruckig_diagnostics_clear(
    ruckig_diagnostics_t* diagnostics,
    ruckig_result_t result,
    ruckig_diagnostic_scope_t scope
) {
    const size_t caller_size = diagnostics ? diagnostics->struct_size : 0u;
    if (ruckig_diagnostics_validate_or_null(diagnostics) != RUCKIG_WORKING) {
        return;
    }
    if (!diagnostics) {
        return;
    }
    memset(diagnostics, 0, ruckig_diagnostics_min_size(caller_size, sizeof(*diagnostics)));
    diagnostics->struct_size = caller_size;
    diagnostics->result = result;
    diagnostics->scope = scope;
    diagnostics->code = RUCKIG_DIAGNOSTIC_NONE;
}

static inline void ruckig_diagnostics_record(
    ruckig_diagnostics_t* diagnostics,
    ruckig_result_t result,
    ruckig_diagnostic_scope_t scope,
    ruckig_diagnostic_code_t code,
    size_t dof,
    size_t section,
    size_t expected_count,
    size_t actual_count,
    double value,
    double limit
) {
    if (ruckig_diagnostics_validate_or_null(diagnostics) != RUCKIG_WORKING || !diagnostics) {
        return;
    }
    ruckig_diagnostics_clear(diagnostics, result, scope);
    diagnostics->code = code;
    diagnostics->dof = dof;
    diagnostics->section = section;
    diagnostics->expected_count = expected_count;
    diagnostics->actual_count = actual_count;
    diagnostics->value = value;
    diagnostics->limit = limit;
}

static inline ruckig_diagnostic_code_t ruckig_diagnostic_code_from_result(ruckig_result_t result) {
    switch (result) {
    case RUCKIG_WORKING:
    case RUCKIG_FINISHED:
        return RUCKIG_DIAGNOSTIC_NONE;
    case RUCKIG_ERROR_ZERO_LIMITS:
        return RUCKIG_DIAGNOSTIC_ZERO_LIMIT;
    case RUCKIG_ERROR_TRAJECTORY_DURATION:
        return RUCKIG_DIAGNOSTIC_TRAJECTORY_DURATION;
    case RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION:
        return RUCKIG_DIAGNOSTIC_SYNCHRONIZATION;
    case RUCKIG_ERROR_UNSUPPORTED:
        return RUCKIG_DIAGNOSTIC_UNSUPPORTED;
    default:
        return RUCKIG_DIAGNOSTIC_UNSUPPORTED;
    }
}

typedef struct ruckig_waypoint_branch {
    size_t index;
    bool acceleration;
    double delta;
    double lower_bound;
} ruckig_waypoint_branch_t;

typedef enum ruckig_waypoint_engine_phase {
    RUCKIG_WAYPOINT_ENGINE_PHASE_IDLE = 0,
    RUCKIG_WAYPOINT_ENGINE_PHASE_BASELINE,
    RUCKIG_WAYPOINT_ENGINE_PHASE_FINITE_DIFFERENCE_035,
    RUCKIG_WAYPOINT_ENGINE_PHASE_FINITE_DIFFERENCE_070,
    RUCKIG_WAYPOINT_ENGINE_PHASE_REFINE_INIT,
    RUCKIG_WAYPOINT_ENGINE_PHASE_REFINE,
    RUCKIG_WAYPOINT_ENGINE_PHASE_BRANCH_INIT,
    RUCKIG_WAYPOINT_ENGINE_PHASE_BRANCH,
    RUCKIG_WAYPOINT_ENGINE_PHASE_COMPLETE
} ruckig_waypoint_engine_phase_t;

typedef struct ruckig_waypoint_optimizer_engine {
    struct ruckig_trajectory* scratch_trajectory;
    double* candidate_velocity;
    double* candidate_acceleration;
    double* best_velocity;
    double* best_acceleration;
    double* baseline_velocity;
    double* baseline_acceleration;
    struct ruckig_input* identity_input;
    ruckig_waypoint_branch_t* branch_queue;
    bool active;
    bool complete;
    bool found;
    bool initial_calculation;
    bool has_published_candidate;
    ruckig_waypoint_engine_phase_t phase;
    double best_duration;
    double baseline_duration;
    double published_duration;
    size_t refine_pass;
    size_t refine_waypoint;
    size_t refine_dof;
    size_t refine_component;
    size_t refine_attempt;
    double refine_original;
    bool refine_original_valid;
    bool refine_improved;
    double branch_scale;
    size_t branch_iteration;
    size_t branch_count;
    size_t branch_index;
    bool branch_queue_valid;
    bool branch_improved_any;
    double last_baseline_duration;
    double last_best_duration;
    double last_best_lower_bound;
    size_t last_candidate_evaluations;
    bool last_improved_baseline;
#ifdef RUCKIG_C_TESTING
    ruckig_result_t test_publish_override_result;
#endif
} ruckig_waypoint_optimizer_engine_t;

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

static inline ruckig_control_interface_t ruckig_effective_control_interface(const ruckig_input_t* input, size_t dof) {
    return input->has_per_dof_control_interface ? input->per_dof_control_interface[dof] : input->control_interface;
}

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
    struct ruckig_trajectory* no_waypoint_scratch_trajectory;
    struct ruckig_input* waypoint_section_input;
    struct ruckig_trajectory* waypoint_section_trajectory;
    ruckig_waypoint_optimizer_engine_t waypoint_engine;
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

struct ruckig_tracking_sequence_continuation {
    size_t dofs;
    size_t capacity;
    size_t target_count;
    size_t completed_count;
    bool active;
    bool was_interrupted;
    bool complete;
    bool optimized_step_active;
    bool optimized_improved;
    double delta_time;
    ruckig_tracking_mode_t mode;
    ruckig_tracking_optimized_strategy_t optimized_strategy;
    double reactiveness;
    size_t look_ahead_cycles;
    size_t max_optimized_candidates;
    size_t optimized_phase;
    size_t optimized_index;
    size_t optimized_window_count;
    double optimized_fast_score;
    double optimized_fast_terminal_position_error;
    double optimized_best_score;
    struct ruckig_input* input;
    struct ruckig_target_state_sequence* target_sequence;
    struct ruckig_tracking_output_sequence* output_prefix;
    ruckig_tracking_diagnostics_t diagnostics;
    ruckig_tracking_diagnostics_t optimized_step_diagnostics;
    double* optimized_best_position;
    double* optimized_best_velocity;
    double* optimized_best_acceleration;
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
    const ruckig_t* otg,
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
#ifdef RUCKIG_C_TESTING
ruckig_result_t ruckig_test_waypoint_engine_step(
    ruckig_t* otg,
    const ruckig_input_t* input,
    bool* candidate_evaluated
);
#endif

#endif
