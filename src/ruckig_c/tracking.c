#include "ruckig_c/tracking_internal.h"

#include <float.h>
#include <math.h>
#include <string.h>

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

size_t min_size(size_t lhs, size_t rhs) {
    return lhs < rhs ? lhs : rhs;
}

const tracking_strategy_config_t* tracking_strategy_config(ruckig_tracking_optimized_strategy_t strategy) {
    size_t i;
    for (i = 0; i < sizeof(tracking_strategy_configs) / sizeof(tracking_strategy_configs[0]); ++i) {
        if (tracking_strategy_configs[i].strategy == strategy) {
            return &tracking_strategy_configs[i];
        }
    }
    return NULL;
}

bool valid_tracking_strategy(ruckig_tracking_optimized_strategy_t strategy) {
    return tracking_strategy_config(strategy) != NULL;
}

void tracking_reset_diagnostics(ruckig_tracking_t* tracking) {
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

void tracking_sync_legacy_diagnostics(ruckig_tracking_t* tracking) {
    if (!tracking) {
        return;
    }
    tracking->last_calculation_status = tracking->diagnostics.calculation_status;
    tracking->last_candidate_count = tracking->diagnostics.candidate_count;
}

void tracking_set_diagnostic_status(
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

void tracking_mark_step_status(
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

void tracking_finalize_score_diagnostics(ruckig_tracking_t* tracking) {
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

void tracking_note_candidate_family(
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

void tracking_note_valid_candidate(ruckig_tracking_t* tracking) {
    if (tracking) {
        ++tracking->diagnostics.valid_candidate_count;
        if (tracking->audit_last_candidate_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT) {
            ++tracking->audit_family_valid[tracking->audit_last_candidate_family];
        }
    }
}

void tracking_note_rejected_candidate(ruckig_tracking_t* tracking) {
    if (tracking) {
        ++tracking->diagnostics.rejected_candidate_count;
    }
}

void tracking_note_budget_exhausted(ruckig_tracking_t* tracking) {
    if (tracking) {
        ++tracking->diagnostics.budget_exhausted_count;
    }
}

void tracking_accumulate_diagnostics(
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

bool finite_vector(const double* values, size_t count) {
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
    value->position = ruckig_allocate_double_vector(dofs);
    value->velocity = ruckig_allocate_double_vector(dofs);
    value->acceleration = ruckig_allocate_double_vector(dofs);
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
    value->position = ruckig_allocate_double_vector(value_count);
    value->velocity = ruckig_allocate_double_vector(value_count);
    value->acceleration = ruckig_allocate_double_vector(value_count);
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
    value->new_position = ruckig_allocate_double_vector(value_count);
    value->new_velocity = ruckig_allocate_double_vector(value_count);
    value->new_acceleration = ruckig_allocate_double_vector(value_count);
    value->new_jerk = ruckig_allocate_double_vector(value_count);
    value->time = ruckig_allocate_double_vector(capacity);
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
    value->optimized_candidate_position = ruckig_allocate_double_vector(dofs);
    value->optimized_candidate_velocity = ruckig_allocate_double_vector(dofs);
    value->optimized_candidate_acceleration = ruckig_allocate_double_vector(dofs);
    value->optimized_candidate_jerk = ruckig_allocate_double_vector(dofs);
    value->optimized_best_position = ruckig_allocate_double_vector(dofs);
    value->optimized_best_velocity = ruckig_allocate_double_vector(dofs);
    value->optimized_best_acceleration = ruckig_allocate_double_vector(dofs);
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
