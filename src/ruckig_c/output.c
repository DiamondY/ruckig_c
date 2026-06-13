#include "ruckig_c/internal.h"

#include <string.h>

static ruckig_result_t ruckig_output_create_impl(
    ruckig_output_t** output,
    size_t dofs,
    size_t max_number_of_waypoints
) {
    ruckig_output_t* value;
    if (!output || dofs == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    *output = NULL;
    value = (ruckig_output_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }

    value->dofs = dofs;
    value->new_position = ruckig_allocate_double_vector(dofs);
    value->new_velocity = ruckig_allocate_double_vector(dofs);
    value->new_acceleration = ruckig_allocate_double_vector(dofs);
    value->new_jerk = ruckig_allocate_double_vector(dofs);
    if (!value->new_position || !value->new_velocity || !value->new_acceleration || !value->new_jerk) {
        ruckig_output_destroy(value);
        return RUCKIG_ERROR;
    }
    if (ruckig_trajectory_create_with_waypoints(&value->trajectory, dofs, max_number_of_waypoints) != RUCKIG_WORKING) {
        ruckig_output_destroy(value);
        return RUCKIG_ERROR;
    }

    *output = value;
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_output_create(ruckig_output_t** output, size_t dofs) {
    return ruckig_output_create_impl(output, dofs, 0);
}

RUCKIG_C_API ruckig_result_t ruckig_output_create_with_waypoints(
    ruckig_output_t** output,
    size_t dofs,
    size_t max_number_of_waypoints
) {
    return ruckig_output_create_impl(output, dofs, max_number_of_waypoints);
}

RUCKIG_C_API void ruckig_output_destroy(ruckig_output_t* output) {
    if (!output) {
        return;
    }
    ruckig_trajectory_destroy(output->trajectory);
    ruckig_free(output->new_position);
    ruckig_free(output->new_velocity);
    ruckig_free(output->new_acceleration);
    ruckig_free(output->new_jerk);
    ruckig_free(output);
}

RUCKIG_C_API void ruckig_output_pass_to_input(
    const ruckig_output_t* output,
    ruckig_input_t* input
) {
    if (!output || !input || output->dofs != input->dofs) {
        return;
    }
    memcpy(input->current_position, output->new_position, sizeof(double) * input->dofs);
    memcpy(input->current_velocity, output->new_velocity, sizeof(double) * input->dofs);
    memcpy(input->current_acceleration, output->new_acceleration, sizeof(double) * input->dofs);
}

RUCKIG_C_API size_t ruckig_output_get_dof_count(const ruckig_output_t* output) {
    return output ? output->dofs : 0;
}

RUCKIG_C_API const double* ruckig_output_new_position_data(const ruckig_output_t* output) {
    return output ? output->new_position : NULL;
}

RUCKIG_C_API const double* ruckig_output_new_velocity_data(const ruckig_output_t* output) {
    return output ? output->new_velocity : NULL;
}

RUCKIG_C_API const double* ruckig_output_new_acceleration_data(const ruckig_output_t* output) {
    return output ? output->new_acceleration : NULL;
}

RUCKIG_C_API const double* ruckig_output_new_jerk_data(const ruckig_output_t* output) {
    return output ? output->new_jerk : NULL;
}

RUCKIG_C_API double ruckig_output_get_time(const ruckig_output_t* output) {
    return output ? output->time : 0.0;
}

RUCKIG_C_API size_t ruckig_output_get_new_section(const ruckig_output_t* output) {
    return output ? output->new_section : 0;
}

RUCKIG_C_API bool ruckig_output_did_section_change(const ruckig_output_t* output) {
    return output ? output->did_section_change : false;
}

RUCKIG_C_API bool ruckig_output_new_calculation(const ruckig_output_t* output) {
    return output ? output->new_calculation : false;
}

RUCKIG_C_API bool ruckig_output_was_calculation_interrupted(const ruckig_output_t* output) {
    return output ? output->was_calculation_interrupted : false;
}

RUCKIG_C_API double ruckig_output_get_calculation_duration(const ruckig_output_t* output) {
    return output ? output->calculation_duration : 0.0;
}

RUCKIG_C_API const ruckig_trajectory_t* ruckig_output_get_trajectory(const ruckig_output_t* output) {
    return output ? output->trajectory : NULL;
}
