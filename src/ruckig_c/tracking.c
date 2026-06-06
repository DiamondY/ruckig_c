#include "ruckig_c/internal.h"

#include <math.h>
#include <string.h>

static double* allocate_double_vector(size_t count) {
    return (double*)ruckig_calloc(count, sizeof(double));
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
    if (ruckig_create(&value->otg, dofs, delta_time) != RUCKIG_WORKING
        || ruckig_input_create(&value->work_input, dofs) != RUCKIG_WORKING
        || ruckig_output_create(&value->work_output, dofs) != RUCKIG_WORKING) {
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

static ruckig_result_t prepare_tracking_input(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    const ruckig_input_t* input
) {
    size_t dof;
    const double horizon = (double)tracking->look_ahead_cycles * tracking->delta_time * tracking->reactiveness;
    if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
        return RUCKIG_ERROR_UNSUPPORTED;
    }
    if (tracking->mode != RUCKIG_TRACKING_FAST) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!ruckig_input_same_dofs(input, tracking->dofs) || input->control_interface != RUCKIG_CONTROL_POSITION || input->has_per_dof_control_interface) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!finite_vector(target_position, tracking->dofs)
        || !finite_vector(target_velocity, tracking->dofs)
        || !finite_vector(target_acceleration, tracking->dofs)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (input != tracking->work_input) {
        if (ruckig_input_copy_state(input, tracking->work_input) != RUCKIG_WORKING) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
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
        tracking->work_input->target_position[dof] = predicted_position;
        tracking->work_input->target_velocity[dof] = predicted_velocity;
        tracking->work_input->target_acceleration[dof] = a;
    }
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_update(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_t* target_state,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_result_t result;
    if (!tracking || !target_state || !input || !output || target_state->dofs != tracking->dofs || output->dofs != tracking->dofs) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    result = prepare_tracking_input(tracking, target_state->position, target_state->velocity, target_state->acceleration, input);
    if (result != RUCKIG_WORKING) {
        return result;
    }
    return ruckig_update(tracking->otg, tracking->work_input, output);
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_calculate_sequence(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_tracking_output_sequence_t* output_sequence
) {
    size_t step;
    if (!tracking || !target_sequence || !input || !output_sequence
        || target_sequence->dofs != tracking->dofs || output_sequence->dofs != tracking->dofs
        || target_sequence->count == 0 || target_sequence->count > output_sequence->capacity
        || !ruckig_input_same_dofs(input, tracking->dofs)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    output_sequence->count = 0;
    ruckig_reset(tracking->otg);
    if (ruckig_input_copy_state(input, tracking->work_input) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    for (step = 0; step < target_sequence->count; ++step) {
        const size_t offset = step * tracking->dofs;
        ruckig_result_t result = prepare_tracking_input(
            tracking,
            &target_sequence->position[offset],
            &target_sequence->velocity[offset],
            &target_sequence->acceleration[offset],
            tracking->work_input
        );
        if (result != RUCKIG_WORKING) {
            return result;
        }
        result = ruckig_update(tracking->otg, tracking->work_input, tracking->work_output);
        if (result != RUCKIG_WORKING && result != RUCKIG_FINISHED) {
            return result;
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
    return RUCKIG_WORKING;
}
