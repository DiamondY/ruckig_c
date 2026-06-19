#include "ruckig_c/internal.h"

#include <float.h>
#include <math.h>
#include <string.h>

static double* allocate_optional_double_vector(size_t count) {
    return count == 0 ? NULL : (double*)ruckig_calloc(count, sizeof(double));
}

static bool* allocate_bool_vector(size_t count) {
    return (bool*)ruckig_calloc(count, sizeof(bool));
}

static ruckig_control_interface_t* allocate_control_interface_vector(size_t count) {
    return (ruckig_control_interface_t*)ruckig_calloc(count, sizeof(ruckig_control_interface_t));
}

static ruckig_synchronization_t* allocate_synchronization_vector(size_t count) {
    return (ruckig_synchronization_t*)ruckig_calloc(count, sizeof(ruckig_synchronization_t));
}

static bool allocate_input_vectors(
    ruckig_input_t* input,
    size_t sections,
    size_t section_values,
    size_t waypoint_values
) {
    const size_t n = input->dofs;
    /* On partial failure, ruckig_input_destroy reclaims fields that were
       already assigned; this relies on the owning input object being calloc-zeroed. */
    input->current_position = ruckig_allocate_double_vector(n);
    input->current_velocity = ruckig_allocate_double_vector(n);
    input->current_acceleration = ruckig_allocate_double_vector(n);
    input->target_position = ruckig_allocate_double_vector(n);
    input->target_velocity = ruckig_allocate_double_vector(n);
    input->target_acceleration = ruckig_allocate_double_vector(n);
    input->max_velocity = ruckig_allocate_double_vector(n);
    input->max_acceleration = ruckig_allocate_double_vector(n);
    input->max_jerk = ruckig_allocate_double_vector(n);
    input->max_position = ruckig_allocate_double_vector(n);
    input->min_position = ruckig_allocate_double_vector(n);
    input->enabled = allocate_bool_vector(n);
    input->min_velocity = ruckig_allocate_double_vector(n);
    input->min_acceleration = ruckig_allocate_double_vector(n);
    input->per_dof_control_interface = allocate_control_interface_vector(n);
    input->per_dof_synchronization = allocate_synchronization_vector(n);
    input->intermediate_positions = allocate_optional_double_vector(waypoint_values);
    input->per_section_max_velocity = ruckig_allocate_double_vector(section_values);
    input->per_section_min_velocity = ruckig_allocate_double_vector(section_values);
    input->per_section_max_acceleration = ruckig_allocate_double_vector(section_values);
    input->per_section_min_acceleration = ruckig_allocate_double_vector(section_values);
    input->per_section_max_jerk = ruckig_allocate_double_vector(section_values);
    input->per_section_max_position = ruckig_allocate_double_vector(section_values);
    input->per_section_min_position = ruckig_allocate_double_vector(section_values);
    input->per_section_minimum_duration = ruckig_allocate_double_vector(sections);

    return input->current_position && input->current_velocity && input->current_acceleration
        && input->target_position && input->target_velocity && input->target_acceleration
        && input->max_velocity && input->max_acceleration && input->max_jerk
        && input->max_position && input->min_position
        && input->enabled && input->min_velocity && input->min_acceleration
        && input->per_dof_control_interface && input->per_dof_synchronization
        && (waypoint_values == 0 || input->intermediate_positions)
        && input->per_section_max_velocity && input->per_section_min_velocity
        && input->per_section_max_acceleration && input->per_section_min_acceleration
        && input->per_section_max_jerk
        && input->per_section_max_position && input->per_section_min_position
        && input->per_section_minimum_duration;
}

static void initialize_input_defaults(ruckig_input_t* input) {
    size_t i;
    input->control_interface = RUCKIG_CONTROL_POSITION;
    input->synchronization = RUCKIG_SYNCHRONIZATION_TIME;
    input->has_per_dof_control_interface = false;
    input->has_per_dof_synchronization = false;
    input->duration_discretization = RUCKIG_DURATION_CONTINUOUS;
    input->has_min_velocity = false;
    input->has_min_acceleration = false;
    input->has_minimum_duration = false;
    input->minimum_duration = 0.0;
    input->waypoint_count = 0;
    input->has_per_section_max_velocity = false;
    input->has_per_section_min_velocity = false;
    input->has_per_section_max_acceleration = false;
    input->has_per_section_min_acceleration = false;
    input->has_per_section_max_jerk = false;
    input->has_per_section_max_position = false;
    input->has_per_section_min_position = false;
    input->has_per_section_minimum_duration = false;
    input->has_interrupt_calculation_duration = false;
    input->interrupt_calculation_duration = 0.0;

    for (i = 0; i < input->dofs; ++i) {
        input->max_acceleration[i] = INFINITY;
        input->max_jerk[i] = INFINITY;
        input->max_position[i] = INFINITY;
        input->min_position[i] = -INFINITY;
        input->enabled[i] = true;
        input->per_dof_control_interface[i] = RUCKIG_CONTROL_POSITION;
        input->per_dof_synchronization[i] = RUCKIG_SYNCHRONIZATION_TIME;
    }
}

static ruckig_result_t ruckig_input_create_impl(
    ruckig_input_t** input,
    size_t dofs,
    size_t max_number_of_waypoints
) {
    ruckig_input_t* value;
    size_t sections = 0;
    size_t section_values = 0;
    size_t waypoint_values = 0;
    if (!input || dofs == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    *input = NULL;
    if (!ruckig_checked_waypoint_counts(dofs, max_number_of_waypoints, &sections, &section_values, &waypoint_values)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    value = (ruckig_input_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }

    value->dofs = dofs;
    value->max_number_of_waypoints = max_number_of_waypoints;
    if (!allocate_input_vectors(value, sections, section_values, waypoint_values)) {
        ruckig_input_destroy(value);
        return RUCKIG_ERROR;
    }
    initialize_input_defaults(value);

    *input = value;
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_input_create(ruckig_input_t** input, size_t dofs) {
    return ruckig_input_create_impl(input, dofs, 0);
}

RUCKIG_C_API ruckig_result_t ruckig_input_create_with_waypoints(
    ruckig_input_t** input,
    size_t dofs,
    size_t max_number_of_waypoints
) {
    return ruckig_input_create_impl(input, dofs, max_number_of_waypoints);
}

RUCKIG_C_API void ruckig_input_destroy(ruckig_input_t* input) {
    if (!input) {
        return;
    }

    ruckig_free(input->current_position);
    ruckig_free(input->current_velocity);
    ruckig_free(input->current_acceleration);
    ruckig_free(input->target_position);
    ruckig_free(input->target_velocity);
    ruckig_free(input->target_acceleration);
    ruckig_free(input->max_velocity);
    ruckig_free(input->max_acceleration);
    ruckig_free(input->max_jerk);
    ruckig_free(input->max_position);
    ruckig_free(input->min_position);
    ruckig_free(input->enabled);
    ruckig_free(input->min_velocity);
    ruckig_free(input->min_acceleration);
    ruckig_free(input->per_dof_control_interface);
    ruckig_free(input->per_dof_synchronization);
    ruckig_free(input->intermediate_positions);
    ruckig_free(input->per_section_max_velocity);
    ruckig_free(input->per_section_min_velocity);
    ruckig_free(input->per_section_max_acceleration);
    ruckig_free(input->per_section_min_acceleration);
    ruckig_free(input->per_section_max_jerk);
    ruckig_free(input->per_section_max_position);
    ruckig_free(input->per_section_min_position);
    ruckig_free(input->per_section_minimum_duration);
    ruckig_free(input);
}

RUCKIG_C_API size_t ruckig_input_get_dof_count(const ruckig_input_t* input) {
    return input ? input->dofs : 0;
}

#define MUTABLE_DATA_ACCESSOR(name, field) \
RUCKIG_C_API double* name(ruckig_input_t* input) { return input ? input->field : NULL; }

#define CONST_DATA_ACCESSOR(name, field) \
RUCKIG_C_API const double* name(const ruckig_input_t* input) { return input ? input->field : NULL; }

MUTABLE_DATA_ACCESSOR(ruckig_input_current_position_data, current_position)
MUTABLE_DATA_ACCESSOR(ruckig_input_current_velocity_data, current_velocity)
MUTABLE_DATA_ACCESSOR(ruckig_input_current_acceleration_data, current_acceleration)
MUTABLE_DATA_ACCESSOR(ruckig_input_target_position_data, target_position)
MUTABLE_DATA_ACCESSOR(ruckig_input_target_velocity_data, target_velocity)
MUTABLE_DATA_ACCESSOR(ruckig_input_target_acceleration_data, target_acceleration)
MUTABLE_DATA_ACCESSOR(ruckig_input_max_velocity_data, max_velocity)
MUTABLE_DATA_ACCESSOR(ruckig_input_max_acceleration_data, max_acceleration)
MUTABLE_DATA_ACCESSOR(ruckig_input_max_jerk_data, max_jerk)
MUTABLE_DATA_ACCESSOR(ruckig_input_max_position_data, max_position)
MUTABLE_DATA_ACCESSOR(ruckig_input_min_position_data, min_position)

RUCKIG_C_API bool* ruckig_input_enabled_data(ruckig_input_t* input) {
    return input ? input->enabled : NULL;
}

CONST_DATA_ACCESSOR(ruckig_input_current_position_const_data, current_position)
CONST_DATA_ACCESSOR(ruckig_input_current_velocity_const_data, current_velocity)
CONST_DATA_ACCESSOR(ruckig_input_current_acceleration_const_data, current_acceleration)
CONST_DATA_ACCESSOR(ruckig_input_target_position_const_data, target_position)
CONST_DATA_ACCESSOR(ruckig_input_target_velocity_const_data, target_velocity)
CONST_DATA_ACCESSOR(ruckig_input_target_acceleration_const_data, target_acceleration)
CONST_DATA_ACCESSOR(ruckig_input_max_velocity_const_data, max_velocity)
CONST_DATA_ACCESSOR(ruckig_input_max_acceleration_const_data, max_acceleration)
CONST_DATA_ACCESSOR(ruckig_input_max_jerk_const_data, max_jerk)
CONST_DATA_ACCESSOR(ruckig_input_max_position_const_data, max_position)
CONST_DATA_ACCESSOR(ruckig_input_min_position_const_data, min_position)

RUCKIG_C_API const bool* ruckig_input_enabled_const_data(const ruckig_input_t* input) {
    return input ? input->enabled : NULL;
}

static bool input_has_full_dof_array(const ruckig_input_t* input, const void* values, size_t count) {
    return input && values && count == input->dofs && count > 0;
}

static bool input_has_dof_index(const ruckig_input_t* input, size_t dof) {
    return input && dof < input->dofs;
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_control_interface(
    ruckig_input_t* input,
    ruckig_control_interface_t control_interface
) {
    if (!input || (control_interface != RUCKIG_CONTROL_POSITION && control_interface != RUCKIG_CONTROL_VELOCITY)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    input->control_interface = control_interface;
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_synchronization(
    ruckig_input_t* input,
    ruckig_synchronization_t synchronization
) {
    if (!input || synchronization < RUCKIG_SYNCHRONIZATION_TIME || synchronization > RUCKIG_SYNCHRONIZATION_NONE) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    input->synchronization = synchronization;
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_per_dof_control_interface(
    ruckig_input_t* input,
    const ruckig_control_interface_t* values,
    size_t count
) {
    size_t i;
    if (!input_has_full_dof_array(input, values, count)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    for (i = 0; i < count; ++i) {
        if (values[i] != RUCKIG_CONTROL_POSITION && values[i] != RUCKIG_CONTROL_VELOCITY) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
    }
    memcpy(input->per_dof_control_interface, values, sizeof(ruckig_control_interface_t) * count);
    input->has_per_dof_control_interface = true;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_input_clear_per_dof_control_interface(ruckig_input_t* input) {
    if (input) {
        input->has_per_dof_control_interface = false;
    }
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_per_dof_synchronization(
    ruckig_input_t* input,
    const ruckig_synchronization_t* values,
    size_t count
) {
    size_t i;
    if (!input_has_full_dof_array(input, values, count)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    for (i = 0; i < count; ++i) {
        if (values[i] < RUCKIG_SYNCHRONIZATION_TIME || values[i] > RUCKIG_SYNCHRONIZATION_NONE) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
    }
    memcpy(input->per_dof_synchronization, values, sizeof(ruckig_synchronization_t) * count);
    input->has_per_dof_synchronization = true;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_input_clear_per_dof_synchronization(ruckig_input_t* input) {
    if (input) {
        input->has_per_dof_synchronization = false;
    }
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_duration_discretization(
    ruckig_input_t* input,
    ruckig_duration_discretization_t duration_discretization
) {
    if (!input || (duration_discretization != RUCKIG_DURATION_CONTINUOUS && duration_discretization != RUCKIG_DURATION_DISCRETE)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    input->duration_discretization = duration_discretization;
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_dof_enabled(
    ruckig_input_t* input,
    size_t dof,
    bool enabled
) {
    if (!input_has_dof_index(input, dof)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    input->enabled[dof] = enabled;
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_min_velocity(
    ruckig_input_t* input,
    const double* min_velocity,
    size_t count
) {
    if (!input_has_full_dof_array(input, min_velocity, count)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    memcpy(input->min_velocity, min_velocity, sizeof(double) * count);
    input->has_min_velocity = true;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_input_clear_min_velocity(ruckig_input_t* input) {
    if (input) {
        input->has_min_velocity = false;
    }
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_min_acceleration(
    ruckig_input_t* input,
    const double* min_acceleration,
    size_t count
) {
    if (!input_has_full_dof_array(input, min_acceleration, count)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    memcpy(input->min_acceleration, min_acceleration, sizeof(double) * count);
    input->has_min_acceleration = true;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_input_clear_min_acceleration(ruckig_input_t* input) {
    if (input) {
        input->has_min_acceleration = false;
    }
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_minimum_duration(
    ruckig_input_t* input,
    double minimum_duration
) {
    if (!input || isnan(minimum_duration) || minimum_duration < 0.0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    input->minimum_duration = minimum_duration;
    input->has_minimum_duration = true;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_input_clear_minimum_duration(ruckig_input_t* input) {
    if (input) {
        input->has_minimum_duration = false;
    }
}

static void clear_per_section_constraints(ruckig_input_t* input) {
    input->has_per_section_max_velocity = false;
    input->has_per_section_min_velocity = false;
    input->has_per_section_max_acceleration = false;
    input->has_per_section_min_acceleration = false;
    input->has_per_section_max_jerk = false;
    input->has_per_section_max_position = false;
    input->has_per_section_min_position = false;
    input->has_per_section_minimum_duration = false;
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_intermediate_positions(
    ruckig_input_t* input,
    const double* flat_positions,
    size_t waypoint_count,
    size_t dofs
) {
    size_t count = 0;
    if (!input || dofs != input->dofs || waypoint_count > input->max_number_of_waypoints
        || !ruckig_checked_mul_size(waypoint_count, dofs, &count)
        || (!flat_positions && count > 0)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (input->waypoint_count != waypoint_count) {
        clear_per_section_constraints(input);
    }
    if (count > 0) {
        memcpy(input->intermediate_positions, flat_positions, sizeof(double) * count);
    }
    input->waypoint_count = waypoint_count;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_input_clear_intermediate_positions(ruckig_input_t* input) {
    if (input) {
        input->waypoint_count = 0;
        clear_per_section_constraints(input);
    }
}

RUCKIG_C_API size_t ruckig_input_get_intermediate_position_count(const ruckig_input_t* input) {
    return input ? input->waypoint_count : 0;
}

RUCKIG_C_API ruckig_result_t ruckig_input_get_intermediate_positions(
    const ruckig_input_t* input,
    double* flat_positions,
    size_t capacity
) {
    size_t count = 0;
    if (!input || !ruckig_checked_mul_size(input->waypoint_count, input->dofs, &count)
        || (!flat_positions && count > 0) || capacity < count) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (count > 0) {
        memcpy(flat_positions, input->intermediate_positions, sizeof(double) * count);
    }
    return RUCKIG_WORKING;
}

static ruckig_result_t set_per_section_vector(
    ruckig_input_t* input,
    double* dst,
    bool* flag,
    const double* values,
    size_t section_count,
    size_t dofs
) {
    size_t expected_sections = 0;
    size_t count = 0;
    if (!input || !dst || !flag || !values || dofs != input->dofs
        || !ruckig_checked_add_size(input->waypoint_count, 1u, &expected_sections)
        || section_count != expected_sections
        || !ruckig_checked_mul_size(section_count, dofs, &count)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    memcpy(dst, values, sizeof(double) * count);
    *flag = true;
    return RUCKIG_WORKING;
}

static ruckig_result_t get_per_section_vector(
    const ruckig_input_t* input,
    const double* src,
    bool flag,
    double* values,
    size_t capacity
) {
    size_t sections = 0;
    size_t count = 0;
    if (!input || !src || !flag
        || !ruckig_checked_add_size(input->waypoint_count, 1u, &sections)
        || !ruckig_checked_mul_size(sections, input->dofs, &count)
        || (!values && count > 0) || capacity < count) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    memcpy(values, src, sizeof(double) * count);
    return RUCKIG_WORKING;
}

#define DEFINE_PER_SECTION_VECTOR_API(suffix, field, flag) \
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_section_##suffix( \
    ruckig_input_t* input, \
    const double* values, \
    size_t section_count, \
    size_t dofs \
) { \
    return set_per_section_vector(input, input ? input->field : NULL, input ? &input->flag : NULL, values, section_count, dofs); \
} \
RUCKIG_C_API void ruckig_input_clear_per_section_##suffix(ruckig_input_t* input) { \
    if (input) { \
        input->flag = false; \
    } \
} \
RUCKIG_C_API bool ruckig_input_has_per_section_##suffix(const ruckig_input_t* input) { \
    return input ? input->flag : false; \
} \
RUCKIG_C_API ruckig_result_t ruckig_input_get_per_section_##suffix( \
    const ruckig_input_t* input, \
    double* values, \
    size_t capacity \
) { \
    return get_per_section_vector(input, input ? input->field : NULL, input ? input->flag : false, values, capacity); \
}

DEFINE_PER_SECTION_VECTOR_API(max_velocity, per_section_max_velocity, has_per_section_max_velocity)
DEFINE_PER_SECTION_VECTOR_API(min_velocity, per_section_min_velocity, has_per_section_min_velocity)
DEFINE_PER_SECTION_VECTOR_API(max_acceleration, per_section_max_acceleration, has_per_section_max_acceleration)
DEFINE_PER_SECTION_VECTOR_API(min_acceleration, per_section_min_acceleration, has_per_section_min_acceleration)
DEFINE_PER_SECTION_VECTOR_API(max_jerk, per_section_max_jerk, has_per_section_max_jerk)
DEFINE_PER_SECTION_VECTOR_API(max_position, per_section_max_position, has_per_section_max_position)
DEFINE_PER_SECTION_VECTOR_API(min_position, per_section_min_position, has_per_section_min_position)

RUCKIG_C_API ruckig_result_t ruckig_input_set_per_section_minimum_duration(
    ruckig_input_t* input,
    const double* values,
    size_t section_count
) {
    size_t i;
    size_t expected_sections = 0;
    if (!input || !values
        || !ruckig_checked_add_size(input->waypoint_count, 1u, &expected_sections)
        || section_count != expected_sections) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    for (i = 0; i < section_count; ++i) {
        if (isnan(values[i]) || values[i] < 0.0) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
    }
    memcpy(input->per_section_minimum_duration, values, sizeof(double) * section_count);
    input->has_per_section_minimum_duration = true;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_input_clear_per_section_minimum_duration(ruckig_input_t* input) {
    if (input) {
        input->has_per_section_minimum_duration = false;
    }
}

RUCKIG_C_API bool ruckig_input_has_per_section_minimum_duration(const ruckig_input_t* input) {
    return input ? input->has_per_section_minimum_duration : false;
}

RUCKIG_C_API ruckig_result_t ruckig_input_get_per_section_minimum_duration(
    const ruckig_input_t* input,
    double* values,
    size_t capacity
) {
    size_t count = 0;
    if (!input || !input->has_per_section_minimum_duration || !values
        || !ruckig_checked_add_size(input->waypoint_count, 1u, &count)
        || capacity < count) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    memcpy(values, input->per_section_minimum_duration, sizeof(double) * count);
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_input_set_interrupt_calculation_duration(
    ruckig_input_t* input,
    double interrupt_calculation_duration
) {
    if (!input || isnan(interrupt_calculation_duration) || interrupt_calculation_duration < 0.0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    input->interrupt_calculation_duration = interrupt_calculation_duration;
    input->has_interrupt_calculation_duration = true;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_input_clear_interrupt_calculation_duration(ruckig_input_t* input) {
    if (input) {
        input->has_interrupt_calculation_duration = false;
    }
}

static ruckig_control_interface_t effective_control_interface(const ruckig_input_t* input, size_t dof) {
    return input->has_per_dof_control_interface ? input->per_dof_control_interface[dof] : input->control_interface;
}

static double v_at_a_zero(double v0, double a0, double j) {
    return v0 + (a0 * a0) / (2.0 * j);
}

RUCKIG_C_API void ruckig_diagnostics_init(ruckig_diagnostics_t* diagnostics) {
    if (!diagnostics) {
        return;
    }
    memset(diagnostics, 0, sizeof(*diagnostics));
    diagnostics->struct_size = sizeof(*diagnostics);
    diagnostics->result = RUCKIG_WORKING;
    diagnostics->scope = RUCKIG_DIAGNOSTIC_SCOPE_NONE;
    diagnostics->code = RUCKIG_DIAGNOSTIC_NONE;
}

#define RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(code, dof_value, section_value, expected_value, actual_value, failed_value, limit_value) \
    do { \
        ruckig_diagnostics_record( \
            diagnostics, \
            RUCKIG_ERROR_INVALID_INPUT, \
            RUCKIG_DIAGNOSTIC_SCOPE_INPUT, \
            (code), \
            (dof_value), \
            (section_value), \
            (expected_value), \
            (actual_value), \
            (failed_value), \
            (limit_value) \
        ); \
        return RUCKIG_ERROR_INVALID_INPUT; \
    } while (0)

RUCKIG_C_API ruckig_result_t ruckig_validate_input_with_diagnostics(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    bool check_current_state_within_limits,
    bool check_target_state_within_limits,
    ruckig_diagnostics_t* diagnostics
) {
    size_t dof;
    if (ruckig_diagnostics_validate_or_null(diagnostics) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!otg || !input) {
        RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NULL_ARGUMENT, 0u, 0u, 0u, 0u, 0.0, 0.0);
    }
    if (otg->dofs != input->dofs) {
        RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
            RUCKIG_DIAGNOSTIC_DOF_MISMATCH,
            0u,
            0u,
            otg->dofs,
            input->dofs,
            0.0,
            0.0
        );
    }

    if (otg->delta_time <= 0.0 && input->duration_discretization != RUCKIG_DURATION_CONTINUOUS) {
        RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
            RUCKIG_DIAGNOSTIC_UNSUPPORTED,
            0u,
            0u,
            0u,
            0u,
            otg->delta_time,
            0.0
        );
    }

    if (input->waypoint_count > 0) {
        if (input->waypoint_count > input->max_number_of_waypoints || input->waypoint_count > otg->max_number_of_waypoints) {
            const size_t input_capacity = input->max_number_of_waypoints < otg->max_number_of_waypoints
                ? input->max_number_of_waypoints
                : otg->max_number_of_waypoints;
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                RUCKIG_DIAGNOSTIC_CAPACITY_MISMATCH,
                0u,
                0u,
                input_capacity,
                input->waypoint_count,
                0.0,
                0.0
            );
        }
        if (input->control_interface != RUCKIG_CONTROL_POSITION
            || input->duration_discretization != RUCKIG_DURATION_CONTINUOUS
            || input->has_minimum_duration
            || input->has_per_dof_control_interface
            || input->has_per_dof_synchronization) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_UNSUPPORTED, 0u, 0u, 0u, 0u, 0.0, 0.0);
        }
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        const double j_max = input->max_jerk[dof];
        const double a_max = input->max_acceleration[dof];
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        const double a0 = input->current_acceleration[dof];
        const double af = input->target_acceleration[dof];
        const double v0 = input->current_velocity[dof];
        const double vf = input->target_velocity[dof];
        const ruckig_control_interface_t control_interface = effective_control_interface(input, dof);

        if (isnan(j_max)) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, j_max, 0.0);
        }
        if (j_max < 0.0) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT, dof, 0u, 0u, 0u, j_max, 0.0);
        }
        if (isnan(a_max)) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, a_max, 0.0);
        }
        if (a_max < 0.0) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT, dof, 0u, 0u, 0u, a_max, 0.0);
        }
        if (isnan(a_min)) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, a_min, 0.0);
        }
        if (a_min > 0.0) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT, dof, 0u, 0u, 0u, a_min, 0.0);
        }
        if (input->waypoint_count > 0 && isinf(j_max)) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, j_max, 0.0);
        }
        if (isnan(a0)) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, a0, 0.0);
        }
        if (isnan(af)) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, af, 0.0);
        }
        if (isnan(v0)) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, v0, 0.0);
        }
        if (isnan(vf)) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, vf, 0.0);
        }
        if (check_current_state_within_limits && (a0 > a_max || a0 < a_min)) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                RUCKIG_DIAGNOSTIC_CURRENT_STATE_OUT_OF_LIMITS,
                dof,
                0u,
                0u,
                0u,
                a0,
                a0 > a_max ? a_max : a_min
            );
        }
        if (check_target_state_within_limits && (af > a_max || af < a_min)) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                RUCKIG_DIAGNOSTIC_TARGET_STATE_OUT_OF_LIMITS,
                dof,
                0u,
                0u,
                0u,
                af,
                af > a_max ? a_max : a_min
            );
        }
        if (input->waypoint_count > 0 && !input->enabled[dof]) {
            size_t waypoint;
            if (input->target_position[dof] != input->current_position[dof]) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                    RUCKIG_DIAGNOSTIC_UNSUPPORTED,
                    dof,
                    0u,
                    0u,
                    0u,
                    input->target_position[dof],
                    input->current_position[dof]
                );
            }
            for (waypoint = 0; waypoint < input->waypoint_count; ++waypoint) {
                if (input->intermediate_positions[waypoint * input->dofs + dof] != input->current_position[dof]) {
                    RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                        RUCKIG_DIAGNOSTIC_UNSUPPORTED,
                        dof,
                        waypoint,
                        0u,
                        0u,
                        input->intermediate_positions[waypoint * input->dofs + dof],
                        input->current_position[dof]
                    );
                }
            }
        }

        if (control_interface == RUCKIG_CONTROL_POSITION) {
            const double p0 = input->current_position[dof];
            const double pf = input->target_position[dof];
            const double v_max = input->max_velocity[dof];
            const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
            const double p_max = input->max_position[dof];
            const double p_min = input->min_position[dof];

            if (isnan(p0)) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, p0, 0.0);
            }
            if (isnan(pf)) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, pf, 0.0);
            }
            if (isnan(v_max)) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, v_max, 0.0);
            }
            if (v_max < 0.0) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT, dof, 0u, 0u, 0u, v_max, 0.0);
            }
            if (isnan(v_min)) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, 0u, 0u, 0u, v_min, 0.0);
            }
            if (v_min > 0.0) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT, dof, 0u, 0u, 0u, v_min, 0.0);
            }
            if (isnan(p_max) || isnan(p_min)) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                    RUCKIG_DIAGNOSTIC_NONFINITE_VALUE,
                    dof,
                    0u,
                    0u,
                    0u,
                    isnan(p_max) ? p_max : p_min,
                    0.0
                );
            }
            if (p_min > p_max) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_UNSUPPORTED, dof, 0u, 0u, 0u, p_min, p_max);
            }
            if (check_current_state_within_limits && (v0 > v_max || v0 < v_min)) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                    RUCKIG_DIAGNOSTIC_CURRENT_STATE_OUT_OF_LIMITS,
                    dof,
                    0u,
                    0u,
                    0u,
                    v0,
                    v0 > v_max ? v_max : v_min
                );
            }
            if (check_target_state_within_limits && (vf > v_max || vf < v_min)) {
                RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                    RUCKIG_DIAGNOSTIC_TARGET_STATE_OUT_OF_LIMITS,
                    dof,
                    0u,
                    0u,
                    0u,
                    vf,
                    vf > v_max ? v_max : v_min
                );
            }
            if (check_current_state_within_limits) {
                if (a0 > 0.0 && j_max > 0.0 && v_at_a_zero(v0, a0, j_max) > v_max) {
                    RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                        RUCKIG_DIAGNOSTIC_CURRENT_STATE_OUT_OF_LIMITS,
                        dof,
                        0u,
                        0u,
                        0u,
                        v_at_a_zero(v0, a0, j_max),
                        v_max
                    );
                }
                if (a0 < 0.0 && j_max > 0.0 && v_at_a_zero(v0, a0, -j_max) < v_min) {
                    RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                        RUCKIG_DIAGNOSTIC_CURRENT_STATE_OUT_OF_LIMITS,
                        dof,
                        0u,
                        0u,
                        0u,
                        v_at_a_zero(v0, a0, -j_max),
                        v_min
                    );
                }
            }
            if (check_target_state_within_limits) {
                if (af < 0.0 && j_max > 0.0 && v_at_a_zero(vf, af, j_max) > v_max) {
                    RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                        RUCKIG_DIAGNOSTIC_TARGET_STATE_OUT_OF_LIMITS,
                        dof,
                        0u,
                        0u,
                        0u,
                        v_at_a_zero(vf, af, j_max),
                        v_max
                    );
                }
                if (af > 0.0 && j_max > 0.0 && v_at_a_zero(vf, af, -j_max) < v_min) {
                    RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                        RUCKIG_DIAGNOSTIC_TARGET_STATE_OUT_OF_LIMITS,
                        dof,
                        0u,
                        0u,
                        0u,
                        v_at_a_zero(vf, af, -j_max),
                        v_min
                    );
                }
            }
        } else if (control_interface != RUCKIG_CONTROL_VELOCITY) {
            RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                RUCKIG_DIAGNOSTIC_INVALID_ENUM,
                dof,
                0u,
                0u,
                0u,
                (double)control_interface,
                0.0
            );
        }
    }

    if (input->waypoint_count > 0) {
        const size_t section_count = input->waypoint_count + 1;
        size_t section;
        for (section = 0; section < section_count; ++section) {
            for (dof = 0; dof < input->dofs; ++dof) {
                const size_t index = section * input->dofs + dof;
                const double max_velocity = input->has_per_section_max_velocity ? input->per_section_max_velocity[index] : input->max_velocity[dof];
                const double min_velocity = input->has_per_section_min_velocity ? input->per_section_min_velocity[index] : (input->has_min_velocity ? input->min_velocity[dof] : -max_velocity);
                const double max_acceleration = input->has_per_section_max_acceleration ? input->per_section_max_acceleration[index] : input->max_acceleration[dof];
                const double min_acceleration = input->has_per_section_min_acceleration ? input->per_section_min_acceleration[index] : (input->has_min_acceleration ? input->min_acceleration[dof] : -max_acceleration);
                const double max_jerk = input->has_per_section_max_jerk ? input->per_section_max_jerk[index] : input->max_jerk[dof];
                const double max_position = input->has_per_section_max_position ? input->per_section_max_position[index] : input->max_position[dof];
                const double min_position = input->has_per_section_min_position ? input->per_section_min_position[index] : input->min_position[dof];
                if (isnan(max_velocity) || isnan(min_velocity) || isnan(max_acceleration)
                    || isnan(min_acceleration) || isnan(max_jerk) || isinf(max_jerk)
                    || isnan(max_position) || isnan(min_position)) {
                    RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NONFINITE_VALUE, dof, section, 0u, 0u, 0.0, 0.0);
                }
                if (max_velocity < 0.0 || min_velocity > 0.0
                    || max_acceleration < 0.0 || min_acceleration > 0.0
                    || max_jerk <= 0.0) {
                    RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT, dof, section, 0u, 0u, 0.0, 0.0);
                }
                if (min_position > max_position) {
                    RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(RUCKIG_DIAGNOSTIC_UNSUPPORTED, dof, section, 0u, 0u, min_position, max_position);
                }
            }
            if (input->has_per_section_minimum_duration) {
                const double minimum_duration = input->per_section_minimum_duration[section];
                if (isnan(minimum_duration) || minimum_duration < 0.0) {
                    RUCKIG_VALIDATE_DIAGNOSTIC_FAIL(
                        isnan(minimum_duration) ? RUCKIG_DIAGNOSTIC_NONFINITE_VALUE : RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT,
                        0u,
                        section,
                        0u,
                        0u,
                        minimum_duration,
                        0.0
                    );
                }
            }
        }
    }

    ruckig_diagnostics_clear(diagnostics, RUCKIG_WORKING, RUCKIG_DIAGNOSTIC_SCOPE_NONE);
    return RUCKIG_WORKING;
}

#undef RUCKIG_VALIDATE_DIAGNOSTIC_FAIL

RUCKIG_C_API ruckig_result_t ruckig_validate_input(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    bool check_current_state_within_limits,
    bool check_target_state_within_limits
) {
    return ruckig_validate_input_with_diagnostics(
        otg,
        input,
        check_current_state_within_limits,
        check_target_state_within_limits,
        NULL
    );
}

ruckig_result_t ruckig_input_copy_state(const ruckig_input_t* src, ruckig_input_t* dst) {
    const size_t n = src && dst ? src->dofs : 0;
    size_t waypoint_values = 0;
    size_t section_values = 0;
    size_t section_count = 0;
    if (!src || !dst || src->dofs != dst->dofs) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (src->waypoint_count > dst->max_number_of_waypoints) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!ruckig_checked_waypoint_counts(src->dofs, src->waypoint_count, &section_count, &section_values, &waypoint_values)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    memcpy(dst->current_position, src->current_position, sizeof(double) * n);
    memcpy(dst->current_velocity, src->current_velocity, sizeof(double) * n);
    memcpy(dst->current_acceleration, src->current_acceleration, sizeof(double) * n);
    memcpy(dst->target_position, src->target_position, sizeof(double) * n);
    memcpy(dst->target_velocity, src->target_velocity, sizeof(double) * n);
    memcpy(dst->target_acceleration, src->target_acceleration, sizeof(double) * n);
    memcpy(dst->max_velocity, src->max_velocity, sizeof(double) * n);
    memcpy(dst->max_acceleration, src->max_acceleration, sizeof(double) * n);
    memcpy(dst->max_jerk, src->max_jerk, sizeof(double) * n);
    memcpy(dst->max_position, src->max_position, sizeof(double) * n);
    memcpy(dst->min_position, src->min_position, sizeof(double) * n);
    memcpy(dst->enabled, src->enabled, sizeof(bool) * n);
    memcpy(dst->min_velocity, src->min_velocity, sizeof(double) * n);
    memcpy(dst->min_acceleration, src->min_acceleration, sizeof(double) * n);
    memcpy(dst->per_dof_control_interface, src->per_dof_control_interface, sizeof(ruckig_control_interface_t) * n);
    memcpy(dst->per_dof_synchronization, src->per_dof_synchronization, sizeof(ruckig_synchronization_t) * n);
    dst->has_min_velocity = src->has_min_velocity;
    dst->has_min_acceleration = src->has_min_acceleration;
    dst->has_minimum_duration = src->has_minimum_duration;
    dst->has_per_dof_control_interface = src->has_per_dof_control_interface;
    dst->has_per_dof_synchronization = src->has_per_dof_synchronization;
    dst->waypoint_count = src->waypoint_count;
    if (waypoint_values > 0) {
        memcpy(dst->intermediate_positions, src->intermediate_positions, sizeof(double) * waypoint_values);
    }
    memcpy(dst->per_section_max_velocity, src->per_section_max_velocity, sizeof(double) * section_values);
    memcpy(dst->per_section_min_velocity, src->per_section_min_velocity, sizeof(double) * section_values);
    memcpy(dst->per_section_max_acceleration, src->per_section_max_acceleration, sizeof(double) * section_values);
    memcpy(dst->per_section_min_acceleration, src->per_section_min_acceleration, sizeof(double) * section_values);
    memcpy(dst->per_section_max_jerk, src->per_section_max_jerk, sizeof(double) * section_values);
    memcpy(dst->per_section_max_position, src->per_section_max_position, sizeof(double) * section_values);
    memcpy(dst->per_section_min_position, src->per_section_min_position, sizeof(double) * section_values);
    memcpy(dst->per_section_minimum_duration, src->per_section_minimum_duration, sizeof(double) * section_count);
    dst->has_per_section_max_velocity = src->has_per_section_max_velocity;
    dst->has_per_section_min_velocity = src->has_per_section_min_velocity;
    dst->has_per_section_max_acceleration = src->has_per_section_max_acceleration;
    dst->has_per_section_min_acceleration = src->has_per_section_min_acceleration;
    dst->has_per_section_max_jerk = src->has_per_section_max_jerk;
    dst->has_per_section_max_position = src->has_per_section_max_position;
    dst->has_per_section_min_position = src->has_per_section_min_position;
    dst->has_per_section_minimum_duration = src->has_per_section_minimum_duration;
    dst->has_interrupt_calculation_duration = src->has_interrupt_calculation_duration;
    dst->interrupt_calculation_duration = src->interrupt_calculation_duration;
    dst->minimum_duration = src->minimum_duration;
    dst->control_interface = src->control_interface;
    dst->synchronization = src->synchronization;
    dst->duration_discretization = src->duration_discretization;
    return RUCKIG_WORKING;
}

bool ruckig_input_same_dofs(const ruckig_input_t* input, size_t dofs) {
    return input && input->dofs == dofs;
}

static bool double_arrays_equal(const double* lhs, const double* rhs, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static bool bool_arrays_equal(const bool* lhs, const bool* rhs, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static bool control_interface_arrays_equal(const ruckig_control_interface_t* lhs, const ruckig_control_interface_t* rhs, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static bool synchronization_arrays_equal(const ruckig_synchronization_t* lhs, const ruckig_synchronization_t* rhs, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static bool ruckig_input_equals_impl(const ruckig_input_t* lhs, const ruckig_input_t* rhs, bool compare_interrupt) {
    const size_t n = lhs && rhs ? lhs->dofs : 0;
    size_t waypoint_values = 0;
    size_t section_values = 0;
    size_t section_count = 0;
    if (!lhs || !rhs || lhs->dofs != rhs->dofs) {
        return false;
    }
    if (!ruckig_checked_waypoint_counts(lhs->dofs, lhs->waypoint_count, &section_count, &section_values, &waypoint_values)) {
        return false;
    }

    return lhs->control_interface == rhs->control_interface
        && lhs->waypoint_count == rhs->waypoint_count
        && lhs->synchronization == rhs->synchronization
        && lhs->duration_discretization == rhs->duration_discretization
        && lhs->has_min_velocity == rhs->has_min_velocity
        && lhs->has_min_acceleration == rhs->has_min_acceleration
        && lhs->has_minimum_duration == rhs->has_minimum_duration
        && lhs->has_per_dof_control_interface == rhs->has_per_dof_control_interface
        && lhs->has_per_dof_synchronization == rhs->has_per_dof_synchronization
        && lhs->has_per_section_max_velocity == rhs->has_per_section_max_velocity
        && lhs->has_per_section_min_velocity == rhs->has_per_section_min_velocity
        && lhs->has_per_section_max_acceleration == rhs->has_per_section_max_acceleration
        && lhs->has_per_section_min_acceleration == rhs->has_per_section_min_acceleration
        && lhs->has_per_section_max_jerk == rhs->has_per_section_max_jerk
        && lhs->has_per_section_max_position == rhs->has_per_section_max_position
        && lhs->has_per_section_min_position == rhs->has_per_section_min_position
        && lhs->has_per_section_minimum_duration == rhs->has_per_section_minimum_duration
        && (!compare_interrupt || lhs->has_interrupt_calculation_duration == rhs->has_interrupt_calculation_duration)
        && (!lhs->has_minimum_duration || lhs->minimum_duration == rhs->minimum_duration)
        && (!compare_interrupt || !lhs->has_interrupt_calculation_duration || lhs->interrupt_calculation_duration == rhs->interrupt_calculation_duration)
        && double_arrays_equal(lhs->current_position, rhs->current_position, n)
        && double_arrays_equal(lhs->current_velocity, rhs->current_velocity, n)
        && double_arrays_equal(lhs->current_acceleration, rhs->current_acceleration, n)
        && double_arrays_equal(lhs->target_position, rhs->target_position, n)
        && double_arrays_equal(lhs->target_velocity, rhs->target_velocity, n)
        && double_arrays_equal(lhs->target_acceleration, rhs->target_acceleration, n)
        && double_arrays_equal(lhs->max_velocity, rhs->max_velocity, n)
        && double_arrays_equal(lhs->max_acceleration, rhs->max_acceleration, n)
        && double_arrays_equal(lhs->max_jerk, rhs->max_jerk, n)
        && double_arrays_equal(lhs->max_position, rhs->max_position, n)
        && double_arrays_equal(lhs->min_position, rhs->min_position, n)
        && (waypoint_values == 0 || double_arrays_equal(lhs->intermediate_positions, rhs->intermediate_positions, waypoint_values))
        && (!lhs->has_min_velocity || double_arrays_equal(lhs->min_velocity, rhs->min_velocity, n))
        && (!lhs->has_min_acceleration || double_arrays_equal(lhs->min_acceleration, rhs->min_acceleration, n))
        && (!lhs->has_per_dof_control_interface || control_interface_arrays_equal(lhs->per_dof_control_interface, rhs->per_dof_control_interface, n))
        && (!lhs->has_per_dof_synchronization || synchronization_arrays_equal(lhs->per_dof_synchronization, rhs->per_dof_synchronization, n))
        && (!lhs->has_per_section_max_velocity || double_arrays_equal(lhs->per_section_max_velocity, rhs->per_section_max_velocity, section_values))
        && (!lhs->has_per_section_min_velocity || double_arrays_equal(lhs->per_section_min_velocity, rhs->per_section_min_velocity, section_values))
        && (!lhs->has_per_section_max_acceleration || double_arrays_equal(lhs->per_section_max_acceleration, rhs->per_section_max_acceleration, section_values))
        && (!lhs->has_per_section_min_acceleration || double_arrays_equal(lhs->per_section_min_acceleration, rhs->per_section_min_acceleration, section_values))
        && (!lhs->has_per_section_max_jerk || double_arrays_equal(lhs->per_section_max_jerk, rhs->per_section_max_jerk, section_values))
        && (!lhs->has_per_section_max_position || double_arrays_equal(lhs->per_section_max_position, rhs->per_section_max_position, section_values))
        && (!lhs->has_per_section_min_position || double_arrays_equal(lhs->per_section_min_position, rhs->per_section_min_position, section_values))
        && (!lhs->has_per_section_minimum_duration || double_arrays_equal(lhs->per_section_minimum_duration, rhs->per_section_minimum_duration, section_count))
        && bool_arrays_equal(lhs->enabled, rhs->enabled, n);
}

bool ruckig_input_equals(const ruckig_input_t* lhs, const ruckig_input_t* rhs) {
    return ruckig_input_equals_impl(lhs, rhs, true);
}

bool ruckig_input_equals_ignoring_interrupt(const ruckig_input_t* lhs, const ruckig_input_t* rhs) {
    return ruckig_input_equals_impl(lhs, rhs, false);
}
