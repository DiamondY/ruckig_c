#include "ruckig_c/internal.h"

#include <float.h>
#include <math.h>
#include <string.h>

#define RUCKIG_TRACKING_DEFAULT_OPTIMIZED_CANDIDATES 16u
#define RUCKIG_TRACKING_MAX_OPTIMIZED_CANDIDATES 128u
#define RUCKIG_TRACKING_SCORE_EPSILON 1e-12

static double* allocate_double_vector(size_t count) {
    return (double*)ruckig_calloc(count, sizeof(double));
}

static size_t min_size(size_t lhs, size_t rhs) {
    return lhs < rhs ? lhs : rhs;
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
    value->last_calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
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
    return RUCKIG_WORKING;
}

RUCKIG_C_API size_t ruckig_tracking_get_max_optimized_candidates(const ruckig_tracking_t* tracking) {
    return tracking ? tracking->max_optimized_candidates : 0;
}

RUCKIG_C_API ruckig_tracking_calculation_status_t ruckig_tracking_get_last_calculation_status(
    const ruckig_tracking_t* tracking
) {
    return tracking ? tracking->last_calculation_status : RUCKIG_TRACKING_CALCULATION_NONE;
}

RUCKIG_C_API size_t ruckig_tracking_get_last_candidate_count(const ruckig_tracking_t* tracking) {
    return tracking ? tracking->last_candidate_count : 0;
}

static void tracking_mark_error(ruckig_tracking_t* tracking) {
    if (tracking) {
        tracking->last_calculation_status = RUCKIG_TRACKING_CALCULATION_ERROR;
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
        tracking->last_calculation_status = RUCKIG_TRACKING_CALCULATION_ERROR;
        return result;
    }
    tracking->last_calculation_status = success_status;
    return result;
}

static ruckig_result_t score_current_tracking_candidate(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    double* score
) {
    size_t sample;
    ruckig_result_t result;
    double value = 0.0;
    if (!score || target_count == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    result = ruckig_calculate(tracking->otg, tracking->work_input, tracking->work_output->trajectory);
    if (result != RUCKIG_WORKING) {
        return result;
    }
    for (sample = 0; sample < target_count; ++sample) {
        size_t dof;
        size_t section = 0;
        const size_t offset = sample * tracking->dofs;
        const double time = (double)(sample + 1) * tracking->delta_time;
        double weight = 1.0 + (double)sample;
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
            weight *= 4.0;
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
                    position_error * position_error
                    + 0.05 * velocity_error * velocity_error
                    + 0.005 * acceleration_error * acceleration_error
                );
                value += 0.0001 * jerk * jerk;
            }
        }
    }
    *score = value;
    return RUCKIG_WORKING;
}

static ruckig_result_t try_tracking_candidate(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    double* best_score,
    bool* improved
) {
    double score = DBL_MAX;
    ruckig_result_t result;
    if (tracking->last_candidate_count >= tracking->max_optimized_candidates) {
        return RUCKIG_WORKING;
    }
    ++tracking->last_candidate_count;
    copy_candidate_to_work_input(tracking);
    result = score_current_tracking_candidate(
        tracking,
        target_position,
        target_velocity,
        target_acceleration,
        target_count,
        &score
    );
    if (result != RUCKIG_WORKING) {
        return RUCKIG_WORKING;
    }
    if (score + RUCKIG_TRACKING_SCORE_EPSILON < *best_score) {
        *best_score = score;
        *improved = true;
        copy_work_input_target_to_best(tracking);
    }
    return RUCKIG_WORKING;
}

static ruckig_result_t evaluate_optimized_tracking(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    size_t sample;
    double fast_score = DBL_MAX;
    double best_score = DBL_MAX;
    bool improved = false;
    ruckig_result_t result;
    const size_t window_count = min_size(target_count, tracking->look_ahead_cycles);
    if (target_count == 0 || window_count == 0
        || !finite_vector(target_position, target_count * tracking->dofs)
        || !finite_vector(target_velocity, target_count * tracking->dofs)
        || !finite_vector(target_acceleration, target_count * tracking->dofs)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    result = prepare_tracking_base(tracking, input);
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
    tracking->last_candidate_count = 1;
    result = score_current_tracking_candidate(
        tracking,
        target_position,
        target_velocity,
        target_acceleration,
        window_count,
        &fast_score
    );
    if (result != RUCKIG_WORKING) {
        tracking->last_calculation_status = RUCKIG_TRACKING_CALCULATION_ERROR;
        return result;
    }
    best_score = fast_score;
    copy_work_input_target_to_best(tracking);

    for (sample = 0; sample < window_count; ++sample) {
        const size_t offset = sample * tracking->dofs;
        result = set_tracking_candidate_prediction(
            tracking,
            &target_position[offset],
            &target_velocity[offset],
            &target_acceleration[offset],
            0.0
        );
        if (result != RUCKIG_WORKING) {
            return result;
        }
        result = try_tracking_candidate(
            tracking,
            target_position,
            target_velocity,
            target_acceleration,
            window_count,
            &best_score,
            &improved
        );
        if (result != RUCKIG_WORKING) {
            return result;
        }
    }

    for (sample = 0; sample < window_count; ++sample) {
        result = set_tracking_candidate_prediction(
            tracking,
            target_position,
            target_velocity,
            target_acceleration,
            (double)(sample + 1) * tracking->delta_time * tracking->reactiveness
        );
        if (result != RUCKIG_WORKING) {
            return result;
        }
        result = try_tracking_candidate(
            tracking,
            target_position,
            target_velocity,
            target_acceleration,
            window_count,
            &best_score,
            &improved
        );
        if (result != RUCKIG_WORKING) {
            return result;
        }
    }

    {
        const size_t terminal_offset = (window_count - 1) * tracking->dofs;
        const double blend_values[3] = {0.25, 0.5, 0.75};
        size_t blend_index;
        for (blend_index = 0; blend_index < 3; ++blend_index) {
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
                target_position,
                target_velocity,
                target_acceleration,
                window_count,
                &best_score,
                &improved
            );
            if (result != RUCKIG_WORKING) {
                return result;
            }
        }

        for (blend_index = 0; blend_index < 2; ++blend_index) {
            size_t dof;
            const double scale = blend_index == 0 ? 0.5 : 0.0;
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
                target_position,
                target_velocity,
                target_acceleration,
                window_count,
                &best_score,
                &improved
            );
            if (result != RUCKIG_WORKING) {
                return result;
            }
        }
    }

    copy_best_to_work_input(tracking);
    return run_prepared_tracking_update(
        tracking,
        output,
        improved ? RUCKIG_TRACKING_CALCULATION_OPTIMIZED : RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
        true
    );
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_update(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_t* target_state,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_result_t result;
    if (!tracking || !target_state || !input || !output || target_state->dofs != tracking->dofs || output->dofs != tracking->dofs) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    tracking->last_candidate_count = 0;
    if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
        return evaluate_optimized_tracking(
            tracking,
            target_state->position,
            target_state->velocity,
            target_state->acceleration,
            1,
            input,
            output
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
    tracking->last_candidate_count = 1;
    return run_prepared_tracking_update(tracking, output, RUCKIG_TRACKING_CALCULATION_FAST, false);
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_update_with_lookahead(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    if (!tracking || !target_sequence || !input || !output
        || target_sequence->dofs != tracking->dofs || output->dofs != tracking->dofs
        || target_sequence->count == 0 || target_sequence->count > target_sequence->capacity) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    tracking->last_candidate_count = 0;
    if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
        return evaluate_optimized_tracking(
            tracking,
            target_sequence->position,
            target_sequence->velocity,
            target_sequence->acceleration,
            target_sequence->count,
            input,
            output
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
    tracking->last_candidate_count = 1;
    return run_prepared_tracking_update(tracking, output, RUCKIG_TRACKING_CALCULATION_FAST, false);
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_calculate_sequence(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_tracking_output_sequence_t* output_sequence
) {
    size_t step;
    bool optimized_any_fallback = false;
    bool optimized_all_optimized = tracking && tracking->mode == RUCKIG_TRACKING_OPTIMIZED;
    if (!tracking || !target_sequence || !input || !output_sequence
        || target_sequence->dofs != tracking->dofs || output_sequence->dofs != tracking->dofs
        || target_sequence->count == 0 || target_sequence->count > output_sequence->capacity
        || !ruckig_input_same_dofs(input, tracking->dofs)) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    tracking->last_candidate_count = 0;
    tracking->last_calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
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
        if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
            const size_t candidates_before = tracking->last_candidate_count;
            result = evaluate_optimized_tracking(
                tracking,
                &target_sequence->position[offset],
                &target_sequence->velocity[offset],
                &target_sequence->acceleration[offset],
                window_count,
                tracking->work_input,
                tracking->work_output
            );
            tracking->last_candidate_count += candidates_before;
        } else if (tracking->mode == RUCKIG_TRACKING_FAST) {
            result = prepare_fast_tracking_input(
                tracking,
                &target_sequence->position[offset],
                &target_sequence->velocity[offset],
                &target_sequence->acceleration[offset],
                tracking->work_input
            );
            if (result == RUCKIG_WORKING) {
                ++tracking->last_candidate_count;
                result = run_prepared_tracking_update(tracking, tracking->work_output, RUCKIG_TRACKING_CALCULATION_FAST, false);
            }
        } else {
            result = RUCKIG_ERROR_INVALID_INPUT;
        }
        if (result != RUCKIG_WORKING && result != RUCKIG_FINISHED) {
            tracking_mark_error(tracking);
            return result;
        }
        if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
            if (tracking->last_calculation_status == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
                optimized_any_fallback = true;
                optimized_all_optimized = false;
            } else if (tracking->last_calculation_status != RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
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
        tracking->last_calculation_status = RUCKIG_TRACKING_CALCULATION_FAST;
    } else if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
        tracking->last_calculation_status = optimized_any_fallback || !optimized_all_optimized
            ? RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK
            : RUCKIG_TRACKING_CALCULATION_OPTIMIZED;
    }
    return RUCKIG_WORKING;
}
