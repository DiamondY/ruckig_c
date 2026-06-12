#include "ruckig_c/internal.h"

#include "ruckig_c/platform_clock.h"

#include <float.h>
#include <math.h>
#include <string.h>

#define RUCKIG_TRACKING_DEFAULT_OPTIMIZED_CANDIDATES 16u
#define RUCKIG_TRACKING_MAX_OPTIMIZED_CANDIDATES 128u
#define RUCKIG_TRACKING_SCORE_EPSILON 1e-12
#define RUCKIG_TRACKING_FAMILY_SCORE_RATIO 0.998
#define RUCKIG_TRACKING_AGGRESSIVE_NEAR_TIE_RATIO 1.01

typedef struct tracking_strategy_config {
    ruckig_tracking_optimized_strategy_t strategy;
    double position_weight;
    double velocity_weight;
    double acceleration_weight;
    double jerk_weight;
    double terminal_weight;
    double horizon_weight_step;
    double acceptance_ratio;
    double candidate_family_score_ratio;
    double near_tie_ratio;
    bool use_terminal_blends;
    bool use_derivative_damping;
    bool use_lead_lag_horizons;
} tracking_strategy_config_t;

typedef struct tracking_interrupt_context {
    bool enabled;
    bool interrupted;
    uint64_t start_us;
    double duration_us;
} tracking_interrupt_context_t;

static tracking_interrupt_context_t tracking_interrupt_context_start(
    const ruckig_input_t* input,
    bool allow_interrupt
) {
    tracking_interrupt_context_t context;
    context.enabled = allow_interrupt && input && input->has_interrupt_calculation_duration;
    context.interrupted = false;
    context.start_us = context.enabled ? ruckig_platform_monotonic_time_us() : 0u;
    context.duration_us = context.enabled ? input->interrupt_calculation_duration : 0.0;
    return context;
}

static bool tracking_interrupt_check(tracking_interrupt_context_t* context) {
    uint64_t now_us;
    double elapsed_us;
    if (!context || !context->enabled || context->interrupted) {
        return false;
    }
    now_us = ruckig_platform_monotonic_time_us();
    elapsed_us = now_us >= context->start_us ? (double)(now_us - context->start_us) : 0.0;
    if (elapsed_us >= context->duration_us) {
        context->interrupted = true;
        return true;
    }
    return false;
}

typedef enum tracking_candidate_family {
    TRACKING_CANDIDATE_FAST,
    TRACKING_CANDIDATE_INSTANTANEOUS,
    TRACKING_CANDIDATE_HORIZON,
    TRACKING_CANDIDATE_TERMINAL_BLEND,
    TRACKING_CANDIDATE_DERIVATIVE_DAMPED,
    TRACKING_CANDIDATE_LEAD_LAG
} tracking_candidate_family_t;

static const tracking_strategy_config_t tracking_strategy_configs[] = {
    {
        RUCKIG_TRACKING_OPTIMIZED_STABLE,
        1.0,
        0.05,
        0.005,
        0.0001,
        4.0,
        1.0,
        1.0,
        RUCKIG_TRACKING_FAMILY_SCORE_RATIO,
        1.0,
        true,
        true,
        false
    },
    {
        RUCKIG_TRACKING_OPTIMIZED_BALANCED,
        1.25,
        0.08,
        0.006,
        0.00008,
        5.0,
        1.25,
        1.0,
        RUCKIG_TRACKING_FAMILY_SCORE_RATIO,
        1.0,
        true,
        true,
        false
    },
    {
        RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE,
        2.0,
        0.10,
        0.004,
        0.00002,
        8.0,
        1.5,
        1.0,
        0.9975,
        RUCKIG_TRACKING_AGGRESSIVE_NEAR_TIE_RATIO,
        true,
        true,
        true
    }
};

static double* allocate_double_vector(size_t count) {
    return (double*)ruckig_calloc(count, sizeof(double));
}

static size_t min_size(size_t lhs, size_t rhs) {
    return lhs < rhs ? lhs : rhs;
}

static const tracking_strategy_config_t* tracking_strategy_config(ruckig_tracking_optimized_strategy_t strategy) {
    size_t i;
    for (i = 0; i < sizeof(tracking_strategy_configs) / sizeof(tracking_strategy_configs[0]); ++i) {
        if (tracking_strategy_configs[i].strategy == strategy) {
            return &tracking_strategy_configs[i];
        }
    }
    return NULL;
}

static bool valid_tracking_strategy(ruckig_tracking_optimized_strategy_t strategy) {
    return tracking_strategy_config(strategy) != NULL;
}

static void tracking_reset_diagnostics(ruckig_tracking_t* tracking) {
    if (!tracking) {
        return;
    }
    memset(&tracking->diagnostics, 0, sizeof(tracking->diagnostics));
    memset(tracking->audit_family_attempted, 0, sizeof(tracking->audit_family_attempted));
    memset(tracking->audit_family_valid, 0, sizeof(tracking->audit_family_valid));
    memset(tracking->audit_family_strict_improved, 0, sizeof(tracking->audit_family_strict_improved));
    memset(tracking->audit_family_near_tie_accepted, 0, sizeof(tracking->audit_family_near_tie_accepted));
    memset(tracking->audit_family_selected, 0, sizeof(tracking->audit_family_selected));
    tracking->diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
    tracking->diagnostics.mode = tracking->mode;
    tracking->diagnostics.optimized_strategy = tracking->optimized_strategy;
    tracking->last_calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
    tracking->last_candidate_count = 0;
    tracking->audit_last_candidate_family = (size_t)TRACKING_CANDIDATE_FAST;
    tracking->audit_best_candidate_family = (size_t)TRACKING_CANDIDATE_FAST;
    tracking->audit_best_candidate_near_tie = false;
    tracking->audit_strict_improved_count = 0;
    tracking->audit_near_tie_accepted_count = 0;
}

static void tracking_sync_legacy_diagnostics(ruckig_tracking_t* tracking) {
    if (!tracking) {
        return;
    }
    tracking->last_calculation_status = tracking->diagnostics.calculation_status;
    tracking->last_candidate_count = tracking->diagnostics.candidate_count;
}

static void tracking_set_diagnostic_status(
    ruckig_tracking_t* tracking,
    ruckig_tracking_calculation_status_t status
) {
    if (!tracking) {
        return;
    }
    tracking->diagnostics.calculation_status = status;
    tracking->diagnostics.mode = tracking->mode;
    tracking->diagnostics.optimized_strategy = tracking->optimized_strategy;
    tracking_sync_legacy_diagnostics(tracking);
}

static void tracking_mark_step_status(
    ruckig_tracking_t* tracking,
    ruckig_tracking_calculation_status_t status
) {
    if (!tracking) {
        return;
    }
    if (status == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
        ++tracking->diagnostics.fallback_step_count;
    } else if (status == RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
        ++tracking->diagnostics.optimized_step_count;
    } else if (status == RUCKIG_TRACKING_CALCULATION_ERROR) {
        ++tracking->diagnostics.error_step_count;
    }
    tracking_set_diagnostic_status(tracking, status);
}

static void tracking_finalize_score_diagnostics(ruckig_tracking_t* tracking) {
    if (!tracking) {
        return;
    }
    if (tracking->diagnostics.fast_score > 0.0) {
        tracking->diagnostics.improvement_ratio =
            (tracking->diagnostics.fast_score - tracking->diagnostics.best_score) / tracking->diagnostics.fast_score;
    } else {
        tracking->diagnostics.improvement_ratio = 0.0;
    }
}

static void tracking_note_candidate_family(
    ruckig_tracking_t* tracking,
    tracking_candidate_family_t family
) {
    if (!tracking) {
        return;
    }
    ++tracking->diagnostics.candidate_count;
    tracking->audit_last_candidate_family = (size_t)family;
    if ((size_t)family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT) {
        ++tracking->audit_family_attempted[(size_t)family];
    }
    switch (family) {
    case TRACKING_CANDIDATE_FAST:
        ++tracking->diagnostics.fast_candidate_count;
        break;
    case TRACKING_CANDIDATE_INSTANTANEOUS:
        ++tracking->diagnostics.instantaneous_candidate_count;
        break;
    case TRACKING_CANDIDATE_HORIZON:
        ++tracking->diagnostics.horizon_candidate_count;
        break;
    case TRACKING_CANDIDATE_TERMINAL_BLEND:
        ++tracking->diagnostics.terminal_blend_candidate_count;
        break;
    case TRACKING_CANDIDATE_DERIVATIVE_DAMPED:
        ++tracking->diagnostics.derivative_damped_candidate_count;
        break;
    case TRACKING_CANDIDATE_LEAD_LAG:
        ++tracking->diagnostics.lead_lag_candidate_count;
        break;
    }
    tracking->last_candidate_count = tracking->diagnostics.candidate_count;
}

static void tracking_note_valid_candidate(ruckig_tracking_t* tracking) {
    if (tracking) {
        ++tracking->diagnostics.valid_candidate_count;
        if (tracking->audit_last_candidate_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT) {
            ++tracking->audit_family_valid[tracking->audit_last_candidate_family];
        }
    }
}

static void tracking_note_rejected_candidate(ruckig_tracking_t* tracking) {
    if (tracking) {
        ++tracking->diagnostics.rejected_candidate_count;
    }
}

static void tracking_note_budget_exhausted(ruckig_tracking_t* tracking) {
    if (tracking) {
        ++tracking->diagnostics.budget_exhausted_count;
    }
}

static void tracking_accumulate_diagnostics(
    ruckig_tracking_diagnostics_t* aggregate,
    const ruckig_tracking_diagnostics_t* step
) {
    if (!aggregate || !step) {
        return;
    }
    aggregate->candidate_count += step->candidate_count;
    aggregate->valid_candidate_count += step->valid_candidate_count;
    aggregate->rejected_candidate_count += step->rejected_candidate_count;
    aggregate->fallback_step_count += step->fallback_step_count;
    aggregate->optimized_step_count += step->optimized_step_count;
    aggregate->error_step_count += step->error_step_count;
    aggregate->budget_exhausted_count += step->budget_exhausted_count;
    aggregate->fast_candidate_count += step->fast_candidate_count;
    aggregate->instantaneous_candidate_count += step->instantaneous_candidate_count;
    aggregate->horizon_candidate_count += step->horizon_candidate_count;
    aggregate->terminal_blend_candidate_count += step->terminal_blend_candidate_count;
    aggregate->derivative_damped_candidate_count += step->derivative_damped_candidate_count;
    aggregate->lead_lag_candidate_count += step->lead_lag_candidate_count;
    aggregate->fast_score += step->fast_score;
    aggregate->best_score += step->best_score;
}

static size_t* allocate_size_vector(size_t count) {
    return (size_t*)ruckig_calloc(count, sizeof(size_t));
}

static ruckig_result_t* allocate_result_vector(size_t count) {
    return (ruckig_result_t*)ruckig_calloc(count, sizeof(ruckig_result_t));
}

static bool valid_value_count(size_t dofs, size_t capacity, size_t* count) {
    if (dofs == 0 || capacity == 0) {
        return false;
    }
    if (capacity > ((size_t)-1) / dofs) {
        return false;
    }
    *count = dofs * capacity;
    return true;
}

static bool finite_vector(const double* values, size_t count) {
    size_t i;
    if (!values) {
        return false;
    }
    for (i = 0; i < count; ++i) {
        if (!isfinite(values[i])) {
            return false;
        }
    }
    return true;
}

RUCKIG_C_API ruckig_result_t ruckig_target_state_create(ruckig_target_state_t** target_state, size_t dofs) {
    ruckig_target_state_t* value;
    if (!target_state || dofs == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    *target_state = NULL;
    value = (ruckig_target_state_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }
    value->dofs = dofs;
    value->position = allocate_double_vector(dofs);
    value->velocity = allocate_double_vector(dofs);
    value->acceleration = allocate_double_vector(dofs);
    if (!value->position || !value->velocity || !value->acceleration) {
        ruckig_target_state_destroy(value);
        return RUCKIG_ERROR;
    }
    *target_state = value;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_target_state_destroy(ruckig_target_state_t* target_state) {
    if (!target_state) {
        return;
    }
    ruckig_free(target_state->position);
    ruckig_free(target_state->velocity);
    ruckig_free(target_state->acceleration);
    ruckig_free(target_state);
}

RUCKIG_C_API size_t ruckig_target_state_get_dof_count(const ruckig_target_state_t* target_state) {
    return target_state ? target_state->dofs : 0;
}

RUCKIG_C_API double* ruckig_target_state_position_data(ruckig_target_state_t* target_state) {
    return target_state ? target_state->position : NULL;
}

RUCKIG_C_API double* ruckig_target_state_velocity_data(ruckig_target_state_t* target_state) {
    return target_state ? target_state->velocity : NULL;
}

RUCKIG_C_API double* ruckig_target_state_acceleration_data(ruckig_target_state_t* target_state) {
    return target_state ? target_state->acceleration : NULL;
}

RUCKIG_C_API const double* ruckig_target_state_position_const_data(const ruckig_target_state_t* target_state) {
    return target_state ? target_state->position : NULL;
}

RUCKIG_C_API const double* ruckig_target_state_velocity_const_data(const ruckig_target_state_t* target_state) {
    return target_state ? target_state->velocity : NULL;
}

RUCKIG_C_API const double* ruckig_target_state_acceleration_const_data(const ruckig_target_state_t* target_state) {
    return target_state ? target_state->acceleration : NULL;
}

RUCKIG_C_API ruckig_result_t ruckig_target_state_sequence_create(
    ruckig_target_state_sequence_t** sequence,
    size_t dofs,
    size_t capacity
) {
    ruckig_target_state_sequence_t* value;
    size_t value_count = 0;
    if (!sequence || !valid_value_count(dofs, capacity, &value_count)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    *sequence = NULL;
    value = (ruckig_target_state_sequence_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }
    value->dofs = dofs;
    value->capacity = capacity;
    value->position = allocate_double_vector(value_count);
    value->velocity = allocate_double_vector(value_count);
    value->acceleration = allocate_double_vector(value_count);
    if (!value->position || !value->velocity || !value->acceleration) {
        ruckig_target_state_sequence_destroy(value);
        return RUCKIG_ERROR;
    }
    *sequence = value;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_target_state_sequence_destroy(ruckig_target_state_sequence_t* sequence) {
    if (!sequence) {
        return;
    }
    ruckig_free(sequence->position);
    ruckig_free(sequence->velocity);
    ruckig_free(sequence->acceleration);
    ruckig_free(sequence);
}

RUCKIG_C_API size_t ruckig_target_state_sequence_get_dof_count(const ruckig_target_state_sequence_t* sequence) {
    return sequence ? sequence->dofs : 0;
}

RUCKIG_C_API size_t ruckig_target_state_sequence_get_capacity(const ruckig_target_state_sequence_t* sequence) {
    return sequence ? sequence->capacity : 0;
}

RUCKIG_C_API size_t ruckig_target_state_sequence_get_count(const ruckig_target_state_sequence_t* sequence) {
    return sequence ? sequence->count : 0;
}

RUCKIG_C_API ruckig_result_t ruckig_target_state_sequence_set_count(ruckig_target_state_sequence_t* sequence, size_t count) {
    if (!sequence || count > sequence->capacity) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    sequence->count = count;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_target_state_sequence_clear(ruckig_target_state_sequence_t* sequence) {
    if (sequence) {
        sequence->count = 0;
    }
}

RUCKIG_C_API double* ruckig_target_state_sequence_position_data(ruckig_target_state_sequence_t* sequence) {
    return sequence ? sequence->position : NULL;
}

RUCKIG_C_API double* ruckig_target_state_sequence_velocity_data(ruckig_target_state_sequence_t* sequence) {
    return sequence ? sequence->velocity : NULL;
}

RUCKIG_C_API double* ruckig_target_state_sequence_acceleration_data(ruckig_target_state_sequence_t* sequence) {
    return sequence ? sequence->acceleration : NULL;
}

RUCKIG_C_API const double* ruckig_target_state_sequence_position_const_data(const ruckig_target_state_sequence_t* sequence) {
    return sequence ? sequence->position : NULL;
}

RUCKIG_C_API const double* ruckig_target_state_sequence_velocity_const_data(const ruckig_target_state_sequence_t* sequence) {
    return sequence ? sequence->velocity : NULL;
}

RUCKIG_C_API const double* ruckig_target_state_sequence_acceleration_const_data(const ruckig_target_state_sequence_t* sequence) {
    return sequence ? sequence->acceleration : NULL;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_output_sequence_create(
    ruckig_tracking_output_sequence_t** sequence,
    size_t dofs,
    size_t capacity
) {
    ruckig_tracking_output_sequence_t* value;
    size_t value_count = 0;
    if (!sequence || !valid_value_count(dofs, capacity, &value_count)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    *sequence = NULL;
    value = (ruckig_tracking_output_sequence_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }
    value->dofs = dofs;
    value->capacity = capacity;
    value->new_position = allocate_double_vector(value_count);
    value->new_velocity = allocate_double_vector(value_count);
    value->new_acceleration = allocate_double_vector(value_count);
    value->new_jerk = allocate_double_vector(value_count);
    value->time = allocate_double_vector(capacity);
    value->section = allocate_size_vector(capacity);
    value->result = allocate_result_vector(capacity);
    if (!value->new_position || !value->new_velocity || !value->new_acceleration || !value->new_jerk
        || !value->time || !value->section || !value->result) {
        ruckig_tracking_output_sequence_destroy(value);
        return RUCKIG_ERROR;
    }
    *sequence = value;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_tracking_output_sequence_destroy(ruckig_tracking_output_sequence_t* sequence) {
    if (!sequence) {
        return;
    }
    ruckig_free(sequence->new_position);
    ruckig_free(sequence->new_velocity);
    ruckig_free(sequence->new_acceleration);
    ruckig_free(sequence->new_jerk);
    ruckig_free(sequence->time);
    ruckig_free(sequence->section);
    ruckig_free(sequence->result);
    ruckig_free(sequence);
}

RUCKIG_C_API size_t ruckig_tracking_output_sequence_get_dof_count(const ruckig_tracking_output_sequence_t* sequence) {
    return sequence ? sequence->dofs : 0;
}

RUCKIG_C_API size_t ruckig_tracking_output_sequence_get_capacity(const ruckig_tracking_output_sequence_t* sequence) {
    return sequence ? sequence->capacity : 0;
}

RUCKIG_C_API size_t ruckig_tracking_output_sequence_get_count(const ruckig_tracking_output_sequence_t* sequence) {
    return sequence ? sequence->count : 0;
}

RUCKIG_C_API void ruckig_tracking_output_sequence_clear(ruckig_tracking_output_sequence_t* sequence) {
    if (sequence) {
        sequence->count = 0;
    }
}

RUCKIG_C_API const double* ruckig_tracking_output_sequence_new_position_const_data(const ruckig_tracking_output_sequence_t* sequence) {
    return sequence ? sequence->new_position : NULL;
}

RUCKIG_C_API const double* ruckig_tracking_output_sequence_new_velocity_const_data(const ruckig_tracking_output_sequence_t* sequence) {
    return sequence ? sequence->new_velocity : NULL;
}

RUCKIG_C_API const double* ruckig_tracking_output_sequence_new_acceleration_const_data(const ruckig_tracking_output_sequence_t* sequence) {
    return sequence ? sequence->new_acceleration : NULL;
}

RUCKIG_C_API const double* ruckig_tracking_output_sequence_new_jerk_const_data(const ruckig_tracking_output_sequence_t* sequence) {
    return sequence ? sequence->new_jerk : NULL;
}

RUCKIG_C_API const double* ruckig_tracking_output_sequence_time_const_data(const ruckig_tracking_output_sequence_t* sequence) {
    return sequence ? sequence->time : NULL;
}

RUCKIG_C_API const size_t* ruckig_tracking_output_sequence_section_const_data(const ruckig_tracking_output_sequence_t* sequence) {
    return sequence ? sequence->section : NULL;
}

RUCKIG_C_API const ruckig_result_t* ruckig_tracking_output_sequence_result_const_data(const ruckig_tracking_output_sequence_t* sequence) {
    return sequence ? sequence->result : NULL;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_create(ruckig_tracking_t** tracking, size_t dofs, double delta_time) {
    ruckig_tracking_t* value;
    if (!tracking || dofs == 0 || !isfinite(delta_time) || delta_time <= 0.0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    *tracking = NULL;
    value = (ruckig_tracking_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }
    value->dofs = dofs;
    value->delta_time = delta_time;
    value->mode = RUCKIG_TRACKING_FAST;
    value->reactiveness = 1.0;
    value->look_ahead_cycles = 1;
    value->max_optimized_candidates = RUCKIG_TRACKING_DEFAULT_OPTIMIZED_CANDIDATES;
    value->optimized_strategy = RUCKIG_TRACKING_OPTIMIZED_BALANCED;
    value->last_calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
    tracking_reset_diagnostics(value);
    if (ruckig_create(&value->otg, dofs, delta_time) != RUCKIG_WORKING
        || ruckig_input_create(&value->work_input, dofs) != RUCKIG_WORKING
        || ruckig_output_create(&value->work_output, dofs) != RUCKIG_WORKING) {
        ruckig_tracking_destroy(value);
        return RUCKIG_ERROR;
    }
    value->optimized_candidate_position = allocate_double_vector(dofs);
    value->optimized_candidate_velocity = allocate_double_vector(dofs);
    value->optimized_candidate_acceleration = allocate_double_vector(dofs);
    value->optimized_candidate_jerk = allocate_double_vector(dofs);
    value->optimized_best_position = allocate_double_vector(dofs);
    value->optimized_best_velocity = allocate_double_vector(dofs);
    value->optimized_best_acceleration = allocate_double_vector(dofs);
    if (!value->optimized_candidate_position || !value->optimized_candidate_velocity
        || !value->optimized_candidate_acceleration || !value->optimized_candidate_jerk
        || !value->optimized_best_position || !value->optimized_best_velocity
        || !value->optimized_best_acceleration) {
        ruckig_tracking_destroy(value);
        return RUCKIG_ERROR;
    }
    *tracking = value;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_tracking_destroy(ruckig_tracking_t* tracking) {
    if (!tracking) {
        return;
    }
    ruckig_output_destroy(tracking->work_output);
    ruckig_input_destroy(tracking->work_input);
    ruckig_destroy(tracking->otg);
    ruckig_free(tracking->optimized_candidate_position);
    ruckig_free(tracking->optimized_candidate_velocity);
    ruckig_free(tracking->optimized_candidate_acceleration);
    ruckig_free(tracking->optimized_candidate_jerk);
    ruckig_free(tracking->optimized_best_position);
    ruckig_free(tracking->optimized_best_velocity);
    ruckig_free(tracking->optimized_best_acceleration);
    ruckig_free(tracking);
}

RUCKIG_C_API size_t ruckig_tracking_get_dof_count(const ruckig_tracking_t* tracking) {
    return tracking ? tracking->dofs : 0;
}

RUCKIG_C_API double ruckig_tracking_get_delta_time(const ruckig_tracking_t* tracking) {
    return tracking ? tracking->delta_time : 0.0;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_set_mode(ruckig_tracking_t* tracking, ruckig_tracking_mode_t mode) {
    if (!tracking || (mode != RUCKIG_TRACKING_FAST && mode != RUCKIG_TRACKING_OPTIMIZED)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    tracking->mode = mode;
    ruckig_reset(tracking->otg);
    tracking_reset_diagnostics(tracking);
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_tracking_mode_t ruckig_tracking_get_mode(const ruckig_tracking_t* tracking) {
    return tracking ? tracking->mode : RUCKIG_TRACKING_FAST;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_set_reactiveness(ruckig_tracking_t* tracking, double reactiveness) {
    if (!tracking || !isfinite(reactiveness) || reactiveness < 0.0 || reactiveness > 1.0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    tracking->reactiveness = reactiveness;
    ruckig_reset(tracking->otg);
    tracking_reset_diagnostics(tracking);
    return RUCKIG_WORKING;
}

RUCKIG_C_API double ruckig_tracking_get_reactiveness(const ruckig_tracking_t* tracking) {
    return tracking ? tracking->reactiveness : 0.0;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_set_look_ahead_cycles(ruckig_tracking_t* tracking, size_t look_ahead_cycles) {
    if (!tracking || look_ahead_cycles == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    tracking->look_ahead_cycles = look_ahead_cycles;
    ruckig_reset(tracking->otg);
    tracking_reset_diagnostics(tracking);
    return RUCKIG_WORKING;
}

RUCKIG_C_API size_t ruckig_tracking_get_look_ahead_cycles(const ruckig_tracking_t* tracking) {
    return tracking ? tracking->look_ahead_cycles : 0;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_set_max_optimized_candidates(
    ruckig_tracking_t* tracking,
    size_t max_candidates
) {
    if (!tracking || max_candidates == 0 || max_candidates > RUCKIG_TRACKING_MAX_OPTIMIZED_CANDIDATES) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    tracking->max_optimized_candidates = max_candidates;
    tracking_reset_diagnostics(tracking);
    return RUCKIG_WORKING;
}

RUCKIG_C_API size_t ruckig_tracking_get_max_optimized_candidates(const ruckig_tracking_t* tracking) {
    return tracking ? tracking->max_optimized_candidates : 0;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_set_optimized_strategy(
    ruckig_tracking_t* tracking,
    ruckig_tracking_optimized_strategy_t strategy
) {
    if (!tracking || !valid_tracking_strategy(strategy)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    tracking->optimized_strategy = strategy;
    ruckig_reset(tracking->otg);
    tracking_reset_diagnostics(tracking);
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_tracking_optimized_strategy_t ruckig_tracking_get_optimized_strategy(
    const ruckig_tracking_t* tracking
) {
    return tracking ? tracking->optimized_strategy : RUCKIG_TRACKING_OPTIMIZED_BALANCED;
}

RUCKIG_C_API ruckig_tracking_calculation_status_t ruckig_tracking_get_last_calculation_status(
    const ruckig_tracking_t* tracking
) {
    return tracking ? tracking->diagnostics.calculation_status : RUCKIG_TRACKING_CALCULATION_NONE;
}

RUCKIG_C_API size_t ruckig_tracking_get_last_candidate_count(const ruckig_tracking_t* tracking) {
    return tracking ? tracking->diagnostics.candidate_count : 0;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_get_last_diagnostics(
    const ruckig_tracking_t* tracking,
    ruckig_tracking_diagnostics_t* diagnostics
) {
    if (!tracking || !diagnostics) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    *diagnostics = tracking->diagnostics;
    return RUCKIG_WORKING;
}

static void tracking_mark_error(ruckig_tracking_t* tracking) {
    if (tracking) {
        tracking_mark_step_status(tracking, RUCKIG_TRACKING_CALCULATION_ERROR);
    }
}

static ruckig_result_t prepare_tracking_base(ruckig_tracking_t* tracking, const ruckig_input_t* input) {
    if (!tracking || !input || !ruckig_input_same_dofs(input, tracking->dofs)
        || input->control_interface != RUCKIG_CONTROL_POSITION || input->has_per_dof_control_interface) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (input != tracking->work_input) {
        if (ruckig_input_copy_state(input, tracking->work_input) != RUCKIG_WORKING) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
    }
    tracking->work_input->has_interrupt_calculation_duration = false;
    tracking->work_input->interrupt_calculation_duration = 0.0;
    return RUCKIG_WORKING;
}

static ruckig_result_t set_tracking_candidate_prediction(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    double horizon
) {
    size_t dof;
    if (!finite_vector(target_position, tracking->dofs)
        || !finite_vector(target_velocity, tracking->dofs)
        || !finite_vector(target_acceleration, tracking->dofs)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!isfinite(horizon) || horizon < 0.0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    for (dof = 0; dof < tracking->dofs; ++dof) {
        const double p = target_position[dof];
        const double v = target_velocity[dof];
        const double a = target_acceleration[dof];
        const double predicted_position = p + v * horizon + 0.5 * a * horizon * horizon;
        const double predicted_velocity = v + a * horizon;
        if (!isfinite(predicted_position) || !isfinite(predicted_velocity)) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
        tracking->optimized_candidate_position[dof] = predicted_position;
        tracking->optimized_candidate_velocity[dof] = predicted_velocity;
        tracking->optimized_candidate_acceleration[dof] = a;
    }
    return RUCKIG_WORKING;
}

static void copy_candidate_to_work_input(ruckig_tracking_t* tracking) {
    memcpy(tracking->work_input->target_position, tracking->optimized_candidate_position, sizeof(double) * tracking->dofs);
    memcpy(tracking->work_input->target_velocity, tracking->optimized_candidate_velocity, sizeof(double) * tracking->dofs);
    memcpy(tracking->work_input->target_acceleration, tracking->optimized_candidate_acceleration, sizeof(double) * tracking->dofs);
}

static void copy_work_input_target_to_best(ruckig_tracking_t* tracking) {
    memcpy(tracking->optimized_best_position, tracking->work_input->target_position, sizeof(double) * tracking->dofs);
    memcpy(tracking->optimized_best_velocity, tracking->work_input->target_velocity, sizeof(double) * tracking->dofs);
    memcpy(tracking->optimized_best_acceleration, tracking->work_input->target_acceleration, sizeof(double) * tracking->dofs);
}

static void copy_best_to_work_input(ruckig_tracking_t* tracking) {
    memcpy(tracking->work_input->target_position, tracking->optimized_best_position, sizeof(double) * tracking->dofs);
    memcpy(tracking->work_input->target_velocity, tracking->optimized_best_velocity, sizeof(double) * tracking->dofs);
    memcpy(tracking->work_input->target_acceleration, tracking->optimized_best_acceleration, sizeof(double) * tracking->dofs);
}

static ruckig_result_t prepare_fast_tracking_input(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    const ruckig_input_t* input
) {
    ruckig_result_t result = prepare_tracking_base(tracking, input);
    if (result != RUCKIG_WORKING) {
        return result;
    }
    result = set_tracking_candidate_prediction(
        tracking,
        target_position,
        target_velocity,
        target_acceleration,
        (double)tracking->look_ahead_cycles * tracking->delta_time * tracking->reactiveness
    );
    if (result != RUCKIG_WORKING) {
        return result;
    }
    copy_candidate_to_work_input(tracking);
    return RUCKIG_WORKING;
}

static ruckig_result_t run_prepared_tracking_update(
    ruckig_tracking_t* tracking,
    ruckig_output_t* output,
    ruckig_tracking_calculation_status_t success_status,
    bool force_reset
) {
    ruckig_result_t result;
    if (force_reset) {
        ruckig_reset(tracking->otg);
    }
    result = ruckig_update(tracking->otg, tracking->work_input, output);
    if (result != RUCKIG_WORKING && result != RUCKIG_FINISHED) {
        tracking_mark_step_status(tracking, RUCKIG_TRACKING_CALCULATION_ERROR);
        return result;
    }
    if (success_status == RUCKIG_TRACKING_CALCULATION_FAST && tracking->diagnostics.candidate_count == 0) {
        tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
        tracking_note_valid_candidate(tracking);
    }
    tracking_mark_step_status(tracking, success_status);
    return result;
}

static ruckig_result_t score_current_tracking_candidate(
    ruckig_tracking_t* tracking,
    const tracking_strategy_config_t* config,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    double* score,
    double* terminal_position_error
) {
    size_t sample;
    ruckig_result_t result;
    double value = 0.0;
    if (!score || !terminal_position_error || target_count == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    *terminal_position_error = 0.0;
    result = ruckig_calculate(tracking->otg, tracking->work_input, tracking->work_output->trajectory);
    if (result != RUCKIG_WORKING) {
        return result;
    }
    for (sample = 0; sample < target_count; ++sample) {
        size_t dof;
        size_t section = 0;
        const size_t offset = sample * tracking->dofs;
        const double time = (double)(sample + 1) * tracking->delta_time;
        double weight = 1.0 + config->horizon_weight_step * (double)sample;
        result = ruckig_trajectory_at_time(
            tracking->work_output->trajectory,
            time,
            tracking->optimized_candidate_position,
            tracking->optimized_candidate_velocity,
            tracking->optimized_candidate_acceleration,
            tracking->optimized_candidate_jerk,
            &section
        );
        (void)section;
        if (result != RUCKIG_WORKING) {
            return result;
        }
        if (sample + 1 == target_count) {
            weight *= config->terminal_weight;
        }
        for (dof = 0; dof < tracking->dofs; ++dof) {
            const double position_error = tracking->optimized_candidate_position[dof] - target_position[offset + dof];
            const double velocity_error = tracking->optimized_candidate_velocity[dof] - target_velocity[offset + dof];
            const double acceleration_error = tracking->optimized_candidate_acceleration[dof] - target_acceleration[offset + dof];
            const double jerk = tracking->optimized_candidate_jerk[dof];
            if (!isfinite(position_error) || !isfinite(velocity_error) || !isfinite(acceleration_error) || !isfinite(jerk)) {
                return RUCKIG_ERROR_INVALID_INPUT;
            }
            if (tracking->work_input->enabled[dof]) {
                value += weight * (
                    config->position_weight * position_error * position_error
                    + config->velocity_weight * velocity_error * velocity_error
                    + config->acceleration_weight * acceleration_error * acceleration_error
                );
                value += weight * config->jerk_weight * jerk * jerk;
                if (sample + 1 == target_count) {
                    *terminal_position_error += position_error * position_error;
                }
            }
        }
    }
    *score = value;
    return RUCKIG_WORKING;
}

static ruckig_result_t try_tracking_candidate(
    ruckig_tracking_t* tracking,
    const tracking_strategy_config_t* config,
    tracking_candidate_family_t family,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    double fast_score,
    double fast_terminal_position_error,
    double* best_score,
    bool* improved
) {
    double raw_score = DBL_MAX;
    double score = DBL_MAX;
    double terminal_position_error = DBL_MAX;
    ruckig_result_t result;
    if (tracking->last_candidate_count >= tracking->max_optimized_candidates) {
        tracking_note_budget_exhausted(tracking);
        return RUCKIG_WORKING;
    }
    tracking_note_candidate_family(tracking, family);
    copy_candidate_to_work_input(tracking);
    result = score_current_tracking_candidate(
        tracking,
        config,
        target_position,
        target_velocity,
        target_acceleration,
        target_count,
        &raw_score,
        &terminal_position_error
    );
    if (result != RUCKIG_WORKING) {
        tracking_note_rejected_candidate(tracking);
        return RUCKIG_WORKING;
    }
    tracking_note_valid_candidate(tracking);
    score = raw_score * (family == TRACKING_CANDIDATE_FAST ? 1.0 : config->candidate_family_score_ratio);
    if (score + RUCKIG_TRACKING_SCORE_EPSILON < *best_score * config->acceptance_ratio) {
        *best_score = score;
        *improved = true;
        if ((size_t)family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT) {
            ++tracking->audit_family_strict_improved[(size_t)family];
        }
        ++tracking->audit_strict_improved_count;
        tracking->audit_best_candidate_family = (size_t)family;
        tracking->audit_best_candidate_near_tie = false;
        copy_work_input_target_to_best(tracking);
    } else if (family != TRACKING_CANDIDATE_FAST
        && config->near_tie_ratio > 1.0
        && raw_score <= fast_score * config->near_tie_ratio
        && terminal_position_error <= 0.5 * fast_terminal_position_error) {
        *best_score = score;
        *improved = true;
        if ((size_t)family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT) {
            ++tracking->audit_family_near_tie_accepted[(size_t)family];
        }
        ++tracking->audit_near_tie_accepted_count;
        tracking->audit_best_candidate_family = (size_t)family;
        tracking->audit_best_candidate_near_tie = true;
        copy_work_input_target_to_best(tracking);
    }
    return RUCKIG_WORKING;
}

static bool tracking_candidate_budget_available(const ruckig_tracking_t* tracking) {
    return tracking->last_candidate_count < tracking->max_optimized_candidates;
}

static ruckig_result_t try_tracking_prediction_candidate(
    ruckig_tracking_t* tracking,
    const tracking_strategy_config_t* config,
    tracking_candidate_family_t family,
    const double* candidate_position,
    const double* candidate_velocity,
    const double* candidate_acceleration,
    double horizon,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    double fast_score,
    double fast_terminal_position_error,
    double* best_score,
    bool* improved
) {
    ruckig_result_t result;
    if (!tracking_candidate_budget_available(tracking)) {
        tracking_note_budget_exhausted(tracking);
        return RUCKIG_WORKING;
    }
    result = set_tracking_candidate_prediction(
        tracking,
        candidate_position,
        candidate_velocity,
        candidate_acceleration,
        horizon
    );
    if (result != RUCKIG_WORKING) {
        return result;
    }
    return try_tracking_candidate(
        tracking,
        config,
        family,
        target_position,
        target_velocity,
        target_acceleration,
        target_count,
        fast_score,
        fast_terminal_position_error,
        best_score,
        improved
    );
}

static ruckig_result_t evaluate_optimized_tracking(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    const ruckig_input_t* input,
    ruckig_output_t* output,
    bool allow_interrupt
) {
    size_t sample;
    double fast_score = DBL_MAX;
    double fast_terminal_position_error = DBL_MAX;
    double best_score = DBL_MAX;
    const tracking_strategy_config_t* config = tracking_strategy_config(tracking->optimized_strategy);
    bool improved = false;
    bool interrupted = false;
    ruckig_result_t result;
    const size_t window_count = min_size(target_count, tracking->look_ahead_cycles);
    tracking_interrupt_context_t interrupt_context = tracking_interrupt_context_start(input, allow_interrupt);
    tracking_reset_diagnostics(tracking);
    if (!config || target_count == 0 || window_count == 0
        || !finite_vector(target_position, target_count * tracking->dofs)
        || !finite_vector(target_velocity, target_count * tracking->dofs)
        || !finite_vector(target_acceleration, target_count * tracking->dofs)) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    result = prepare_tracking_base(tracking, input);
    if (result != RUCKIG_WORKING) {
        tracking_mark_error(tracking);
        return result;
    }

    result = set_tracking_candidate_prediction(
        tracking,
        target_position,
        target_velocity,
        target_acceleration,
        (double)window_count * tracking->delta_time * tracking->reactiveness
    );
    if (result != RUCKIG_WORKING) {
        tracking_mark_error(tracking);
        return result;
    }
    copy_candidate_to_work_input(tracking);
    tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
    result = score_current_tracking_candidate(
        tracking,
        config,
        target_position,
        target_velocity,
        target_acceleration,
        window_count,
        &fast_score,
        &fast_terminal_position_error
    );
    if (result != RUCKIG_WORKING) {
        tracking_note_rejected_candidate(tracking);
        tracking_mark_error(tracking);
        return result;
    }
    tracking_note_valid_candidate(tracking);
    best_score = fast_score;
    tracking->diagnostics.fast_score = fast_score;
    tracking->diagnostics.best_score = best_score;
    copy_work_input_target_to_best(tracking);
    if (tracking_interrupt_check(&interrupt_context)) {
        interrupted = true;
        tracking_note_budget_exhausted(tracking);
        goto finish_optimized_tracking;
    }

    for (sample = 0; sample < window_count; ++sample) {
        const size_t offset = sample * tracking->dofs;
        result = try_tracking_prediction_candidate(
            tracking,
            config,
            TRACKING_CANDIDATE_INSTANTANEOUS,
            &target_position[offset],
            &target_velocity[offset],
            &target_acceleration[offset],
            0.0,
            target_position,
            target_velocity,
            target_acceleration,
            window_count,
            fast_score,
            fast_terminal_position_error,
            &best_score,
            &improved
        );
        if (result != RUCKIG_WORKING) {
            return result;
        }
        if (tracking_interrupt_check(&interrupt_context)) {
            interrupted = true;
            tracking_note_budget_exhausted(tracking);
            goto finish_optimized_tracking;
        }
    }

    for (sample = 0; sample < window_count; ++sample) {
        result = try_tracking_prediction_candidate(
            tracking,
            config,
            TRACKING_CANDIDATE_HORIZON,
            target_position,
            target_velocity,
            target_acceleration,
            (double)(sample + 1) * tracking->delta_time * tracking->reactiveness,
            target_position,
            target_velocity,
            target_acceleration,
            window_count,
            fast_score,
            fast_terminal_position_error,
            &best_score,
            &improved
        );
        if (result != RUCKIG_WORKING) {
            return result;
        }
        if (tracking_interrupt_check(&interrupt_context)) {
            interrupted = true;
            tracking_note_budget_exhausted(tracking);
            goto finish_optimized_tracking;
        }
    }

    if (config->use_lead_lag_horizons) {
        const double horizon_values[4] = {
            0.5 * tracking->delta_time * tracking->reactiveness,
            ((double)window_count + 0.5) * tracking->delta_time * tracking->reactiveness,
            ((double)window_count + 1.0) * tracking->delta_time * tracking->reactiveness,
            ((double)window_count + 2.0) * tracking->delta_time * tracking->reactiveness
        };
        size_t horizon_index;
        for (horizon_index = 0; horizon_index < 4; ++horizon_index) {
            result = try_tracking_prediction_candidate(
                tracking,
                config,
                TRACKING_CANDIDATE_LEAD_LAG,
                target_position,
                target_velocity,
                target_acceleration,
                horizon_values[horizon_index],
                target_position,
                target_velocity,
                target_acceleration,
                window_count,
                fast_score,
                fast_terminal_position_error,
                &best_score,
                &improved
            );
            if (result != RUCKIG_WORKING) {
                return result;
            }
            if (tracking_interrupt_check(&interrupt_context)) {
                interrupted = true;
                tracking_note_budget_exhausted(tracking);
                goto finish_optimized_tracking;
            }
        }
    }

    {
        const size_t terminal_offset = (window_count - 1) * tracking->dofs;
        const double blend_values[4] = {0.25, 0.5, 0.75, 1.0};
        size_t blend_index;
        for (blend_index = 0; config->use_terminal_blends && blend_index < 4; ++blend_index) {
            size_t dof;
            const double blend = blend_values[blend_index];
            result = set_tracking_candidate_prediction(
                tracking,
                target_position,
                target_velocity,
                target_acceleration,
                (double)tracking->look_ahead_cycles * tracking->delta_time * tracking->reactiveness
            );
            if (result != RUCKIG_WORKING) {
                return result;
            }
            for (dof = 0; dof < tracking->dofs; ++dof) {
                tracking->optimized_candidate_position[dof] =
                    (1.0 - blend) * tracking->optimized_candidate_position[dof]
                    + blend * target_position[terminal_offset + dof];
                tracking->optimized_candidate_velocity[dof] =
                    (1.0 - blend) * tracking->optimized_candidate_velocity[dof]
                    + blend * target_velocity[terminal_offset + dof];
                tracking->optimized_candidate_acceleration[dof] =
                    (1.0 - blend) * tracking->optimized_candidate_acceleration[dof]
                    + blend * target_acceleration[terminal_offset + dof];
            }
            result = try_tracking_candidate(
                tracking,
                config,
                TRACKING_CANDIDATE_TERMINAL_BLEND,
                target_position,
                target_velocity,
                target_acceleration,
                window_count,
                fast_score,
                fast_terminal_position_error,
                &best_score,
                &improved
            );
            if (result != RUCKIG_WORKING) {
                return result;
            }
            if (tracking_interrupt_check(&interrupt_context)) {
                interrupted = true;
                tracking_note_budget_exhausted(tracking);
                goto finish_optimized_tracking;
            }
        }

        for (blend_index = 0; config->use_derivative_damping && blend_index < 3; ++blend_index) {
            size_t dof;
            const double scale = blend_index == 0 ? 0.75 : (blend_index == 1 ? 0.5 : 0.0);
            result = set_tracking_candidate_prediction(
                tracking,
                &target_position[terminal_offset],
                &target_velocity[terminal_offset],
                &target_acceleration[terminal_offset],
                0.0
            );
            if (result != RUCKIG_WORKING) {
                return result;
            }
            for (dof = 0; dof < tracking->dofs; ++dof) {
                tracking->optimized_candidate_velocity[dof] *= scale;
                tracking->optimized_candidate_acceleration[dof] *= scale;
            }
            result = try_tracking_candidate(
                tracking,
                config,
                TRACKING_CANDIDATE_DERIVATIVE_DAMPED,
                target_position,
                target_velocity,
                target_acceleration,
                window_count,
                fast_score,
                fast_terminal_position_error,
                &best_score,
                &improved
            );
            if (result != RUCKIG_WORKING) {
                return result;
            }
            if (tracking_interrupt_check(&interrupt_context)) {
                interrupted = true;
                tracking_note_budget_exhausted(tracking);
                goto finish_optimized_tracking;
            }
        }
    }

finish_optimized_tracking:
    copy_best_to_work_input(tracking);
    if (tracking->audit_best_candidate_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT) {
        ++tracking->audit_family_selected[tracking->audit_best_candidate_family];
    }
    tracking->diagnostics.best_score = best_score;
    tracking_finalize_score_diagnostics(tracking);
    result = run_prepared_tracking_update(
        tracking,
        output,
        improved ? RUCKIG_TRACKING_CALCULATION_OPTIMIZED : RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
        true
    );
    if ((result == RUCKIG_WORKING || result == RUCKIG_FINISHED) && interrupted) {
        output->was_calculation_interrupted = true;
    }
    return result;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_update(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_t* target_state,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_result_t result;
    if (tracking) {
        tracking_reset_diagnostics(tracking);
    }
    if (!tracking || !target_state || !input || !output || target_state->dofs != tracking->dofs || output->dofs != tracking->dofs) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
        return evaluate_optimized_tracking(
            tracking,
            target_state->position,
            target_state->velocity,
            target_state->acceleration,
            1,
            input,
            output,
            true
        );
    }
    if (tracking->mode != RUCKIG_TRACKING_FAST) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    result = prepare_fast_tracking_input(tracking, target_state->position, target_state->velocity, target_state->acceleration, input);
    if (result != RUCKIG_WORKING) {
        tracking_mark_error(tracking);
        return result;
    }
    tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
    tracking_note_valid_candidate(tracking);
    return run_prepared_tracking_update(tracking, output, RUCKIG_TRACKING_CALCULATION_FAST, false);
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_update_with_lookahead(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    if (tracking) {
        tracking_reset_diagnostics(tracking);
    }
    if (!tracking || !target_sequence || !input || !output
        || target_sequence->dofs != tracking->dofs || output->dofs != tracking->dofs
        || target_sequence->count == 0 || target_sequence->count > target_sequence->capacity) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
        return evaluate_optimized_tracking(
            tracking,
            target_sequence->position,
            target_sequence->velocity,
            target_sequence->acceleration,
            target_sequence->count,
            input,
            output,
            true
        );
    }
    if (tracking->mode != RUCKIG_TRACKING_FAST) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    {
        ruckig_result_t result = prepare_fast_tracking_input(
            tracking,
            target_sequence->position,
            target_sequence->velocity,
            target_sequence->acceleration,
            input
        );
        if (result != RUCKIG_WORKING) {
            tracking_mark_error(tracking);
            return result;
        }
    }
    tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
    tracking_note_valid_candidate(tracking);
    return run_prepared_tracking_update(tracking, output, RUCKIG_TRACKING_CALCULATION_FAST, false);
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_calculate_sequence(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_tracking_output_sequence_t* output_sequence
) {
    size_t step;
    ruckig_tracking_diagnostics_t aggregate;
    bool optimized_any_fallback = false;
    bool optimized_all_optimized = tracking && tracking->mode == RUCKIG_TRACKING_OPTIMIZED;
    if (tracking) {
        tracking_reset_diagnostics(tracking);
    }
    if (!tracking || !target_sequence || !input || !output_sequence
        || target_sequence->dofs != tracking->dofs || output_sequence->dofs != tracking->dofs
        || target_sequence->count == 0 || target_sequence->count > output_sequence->capacity
        || !ruckig_input_same_dofs(input, tracking->dofs)) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    memset(&aggregate, 0, sizeof(aggregate));
    aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
    aggregate.mode = tracking->mode;
    aggregate.optimized_strategy = tracking->optimized_strategy;
    output_sequence->count = 0;
    ruckig_reset(tracking->otg);
    if (ruckig_input_copy_state(input, tracking->work_input) != RUCKIG_WORKING) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    for (step = 0; step < target_sequence->count; ++step) {
        const size_t offset = step * tracking->dofs;
        const size_t remaining = target_sequence->count - step;
        const size_t window_count = tracking->mode == RUCKIG_TRACKING_OPTIMIZED
            ? min_size(remaining, tracking->look_ahead_cycles)
            : 1;
        ruckig_result_t result;
        tracking_reset_diagnostics(tracking);
        if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
            result = evaluate_optimized_tracking(
                tracking,
                &target_sequence->position[offset],
                &target_sequence->velocity[offset],
                &target_sequence->acceleration[offset],
                window_count,
                tracking->work_input,
                tracking->work_output,
                false
            );
        } else if (tracking->mode == RUCKIG_TRACKING_FAST) {
            result = prepare_fast_tracking_input(
                tracking,
                &target_sequence->position[offset],
                &target_sequence->velocity[offset],
                &target_sequence->acceleration[offset],
                tracking->work_input
            );
            if (result == RUCKIG_WORKING) {
                tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
                tracking_note_valid_candidate(tracking);
                result = run_prepared_tracking_update(tracking, tracking->work_output, RUCKIG_TRACKING_CALCULATION_FAST, false);
            }
        } else {
            result = RUCKIG_ERROR_INVALID_INPUT;
        }
        {
            const bool step_marked_error = tracking->diagnostics.error_step_count > 0;
            tracking_accumulate_diagnostics(&aggregate, &tracking->diagnostics);
            if (result != RUCKIG_WORKING && result != RUCKIG_FINISHED) {
                tracking_mark_error(tracking);
                aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_ERROR;
                if (!step_marked_error) {
                    ++aggregate.error_step_count;
                }
                tracking->diagnostics = aggregate;
                tracking_sync_legacy_diagnostics(tracking);
                return result;
            }
        }
        if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
            if (tracking->diagnostics.calculation_status == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
                optimized_any_fallback = true;
                optimized_all_optimized = false;
            } else if (tracking->diagnostics.calculation_status != RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
                optimized_all_optimized = false;
            }
        }
        memcpy(&output_sequence->new_position[offset], tracking->work_output->new_position, sizeof(double) * tracking->dofs);
        memcpy(&output_sequence->new_velocity[offset], tracking->work_output->new_velocity, sizeof(double) * tracking->dofs);
        memcpy(&output_sequence->new_acceleration[offset], tracking->work_output->new_acceleration, sizeof(double) * tracking->dofs);
        memcpy(&output_sequence->new_jerk[offset], tracking->work_output->new_jerk, sizeof(double) * tracking->dofs);
        output_sequence->time[step] = (double)(step + 1) * tracking->delta_time;
        output_sequence->section[step] = tracking->work_output->new_section;
        output_sequence->result[step] = result;
        output_sequence->count = step + 1;
        ruckig_output_pass_to_input(tracking->work_output, tracking->work_input);
    }
    if (tracking->mode == RUCKIG_TRACKING_FAST) {
        aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_FAST;
    } else if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
        aggregate.calculation_status = optimized_any_fallback || !optimized_all_optimized
            ? RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK
            : RUCKIG_TRACKING_CALCULATION_OPTIMIZED;
    }
    if (aggregate.fast_score > 0.0) {
        aggregate.improvement_ratio = (aggregate.fast_score - aggregate.best_score) / aggregate.fast_score;
    }
    tracking->diagnostics = aggregate;
    tracking_sync_legacy_diagnostics(tracking);
    return RUCKIG_WORKING;
}
