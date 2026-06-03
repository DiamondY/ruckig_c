#include "ruckig_c/internal.h"

#include <float.h>
#include <math.h>
#include <string.h>

static double* allocate_double_vector(size_t count) {
    return (double*)ruckig_calloc(count, sizeof(double));
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

static bool allocate_input_vectors(ruckig_input_t* input) {
    const size_t n = input->dofs;
    input->current_position = allocate_double_vector(n);
    input->current_velocity = allocate_double_vector(n);
    input->current_acceleration = allocate_double_vector(n);
    input->target_position = allocate_double_vector(n);
    input->target_velocity = allocate_double_vector(n);
    input->target_acceleration = allocate_double_vector(n);
    input->max_velocity = allocate_double_vector(n);
    input->max_acceleration = allocate_double_vector(n);
    input->max_jerk = allocate_double_vector(n);
    input->enabled = allocate_bool_vector(n);
    input->min_velocity = allocate_double_vector(n);
    input->min_acceleration = allocate_double_vector(n);
    input->per_dof_control_interface = allocate_control_interface_vector(n);
    input->per_dof_synchronization = allocate_synchronization_vector(n);

    return input->current_position && input->current_velocity && input->current_acceleration
        && input->target_position && input->target_velocity && input->target_acceleration
        && input->max_velocity && input->max_acceleration && input->max_jerk
        && input->enabled && input->min_velocity && input->min_acceleration
        && input->per_dof_control_interface && input->per_dof_synchronization;
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

    for (i = 0; i < input->dofs; ++i) {
        input->max_acceleration[i] = INFINITY;
        input->max_jerk[i] = INFINITY;
        input->enabled[i] = true;
        input->per_dof_control_interface[i] = RUCKIG_CONTROL_POSITION;
        input->per_dof_synchronization[i] = RUCKIG_SYNCHRONIZATION_TIME;
    }
}

ruckig_result_t ruckig_input_create(ruckig_input_t** input, size_t dofs) {
    ruckig_input_t* value;
    if (!input || dofs == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    *input = NULL;
    value = (ruckig_input_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }

    value->dofs = dofs;
    if (!allocate_input_vectors(value)) {
        ruckig_input_destroy(value);
        return RUCKIG_ERROR;
    }
    initialize_input_defaults(value);

    *input = value;
    return RUCKIG_WORKING;
}

void ruckig_input_destroy(ruckig_input_t* input) {
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
    ruckig_free(input->enabled);
    ruckig_free(input->min_velocity);
    ruckig_free(input->min_acceleration);
    ruckig_free(input->per_dof_control_interface);
    ruckig_free(input->per_dof_synchronization);
    ruckig_free(input);
}

size_t ruckig_input_get_dof_count(const ruckig_input_t* input) {
    return input ? input->dofs : 0;
}

#define MUTABLE_DATA_ACCESSOR(name, field) \
double* name(ruckig_input_t* input) { return input ? input->field : NULL; }

#define CONST_DATA_ACCESSOR(name, field) \
const double* name(const ruckig_input_t* input) { return input ? input->field : NULL; }

MUTABLE_DATA_ACCESSOR(ruckig_input_current_position_data, current_position)
MUTABLE_DATA_ACCESSOR(ruckig_input_current_velocity_data, current_velocity)
MUTABLE_DATA_ACCESSOR(ruckig_input_current_acceleration_data, current_acceleration)
MUTABLE_DATA_ACCESSOR(ruckig_input_target_position_data, target_position)
MUTABLE_DATA_ACCESSOR(ruckig_input_target_velocity_data, target_velocity)
MUTABLE_DATA_ACCESSOR(ruckig_input_target_acceleration_data, target_acceleration)
MUTABLE_DATA_ACCESSOR(ruckig_input_max_velocity_data, max_velocity)
MUTABLE_DATA_ACCESSOR(ruckig_input_max_acceleration_data, max_acceleration)
MUTABLE_DATA_ACCESSOR(ruckig_input_max_jerk_data, max_jerk)

bool* ruckig_input_enabled_data(ruckig_input_t* input) {
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

const bool* ruckig_input_enabled_const_data(const ruckig_input_t* input) {
    return input ? input->enabled : NULL;
}

ruckig_result_t ruckig_input_set_control_interface(
    ruckig_input_t* input,
    ruckig_control_interface_t control_interface
) {
    if (!input || (control_interface != RUCKIG_CONTROL_POSITION && control_interface != RUCKIG_CONTROL_VELOCITY)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    input->control_interface = control_interface;
    return RUCKIG_WORKING;
}

ruckig_result_t ruckig_input_set_synchronization(
    ruckig_input_t* input,
    ruckig_synchronization_t synchronization
) {
    if (!input || synchronization < RUCKIG_SYNCHRONIZATION_TIME || synchronization > RUCKIG_SYNCHRONIZATION_NONE) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    input->synchronization = synchronization;
    return RUCKIG_WORKING;
}

ruckig_result_t ruckig_input_set_per_dof_control_interface(
    ruckig_input_t* input,
    const ruckig_control_interface_t* values,
    size_t count
) {
    size_t i;
    if (!input || !values || count != input->dofs || count == 0) {
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

void ruckig_input_clear_per_dof_control_interface(ruckig_input_t* input) {
    if (input) {
        input->has_per_dof_control_interface = false;
    }
}

ruckig_result_t ruckig_input_set_per_dof_synchronization(
    ruckig_input_t* input,
    const ruckig_synchronization_t* values,
    size_t count
) {
    size_t i;
    if (!input || !values || count != input->dofs || count == 0) {
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

void ruckig_input_clear_per_dof_synchronization(ruckig_input_t* input) {
    if (input) {
        input->has_per_dof_synchronization = false;
    }
}

ruckig_result_t ruckig_input_set_duration_discretization(
    ruckig_input_t* input,
    ruckig_duration_discretization_t duration_discretization
) {
    if (!input || (duration_discretization != RUCKIG_DURATION_CONTINUOUS && duration_discretization != RUCKIG_DURATION_DISCRETE)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    input->duration_discretization = duration_discretization;
    return RUCKIG_WORKING;
}

ruckig_result_t ruckig_input_set_dof_enabled(
    ruckig_input_t* input,
    size_t dof,
    bool enabled
) {
    if (!input || dof >= input->dofs) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    input->enabled[dof] = enabled;
    return RUCKIG_WORKING;
}

ruckig_result_t ruckig_input_set_min_velocity(
    ruckig_input_t* input,
    const double* min_velocity,
    size_t count
) {
    if (!input || !min_velocity || count != input->dofs || count == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    memcpy(input->min_velocity, min_velocity, sizeof(double) * count);
    input->has_min_velocity = true;
    return RUCKIG_WORKING;
}

void ruckig_input_clear_min_velocity(ruckig_input_t* input) {
    if (input) {
        input->has_min_velocity = false;
    }
}

ruckig_result_t ruckig_input_set_min_acceleration(
    ruckig_input_t* input,
    const double* min_acceleration,
    size_t count
) {
    if (!input || !min_acceleration || count != input->dofs || count == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    memcpy(input->min_acceleration, min_acceleration, sizeof(double) * count);
    input->has_min_acceleration = true;
    return RUCKIG_WORKING;
}

void ruckig_input_clear_min_acceleration(ruckig_input_t* input) {
    if (input) {
        input->has_min_acceleration = false;
    }
}

ruckig_result_t ruckig_input_set_minimum_duration(
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

void ruckig_input_clear_minimum_duration(ruckig_input_t* input) {
    if (input) {
        input->has_minimum_duration = false;
    }
}

static ruckig_control_interface_t effective_control_interface(const ruckig_input_t* input, size_t dof) {
    return input->has_per_dof_control_interface ? input->per_dof_control_interface[dof] : input->control_interface;
}

static double v_at_a_zero(double v0, double a0, double j) {
    return v0 + (a0 * a0) / (2.0 * j);
}

ruckig_result_t ruckig_validate_input(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    bool check_current_state_within_limits,
    bool check_target_state_within_limits
) {
    size_t dof;
    if (!otg || !input || otg->dofs != input->dofs) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    if (otg->delta_time <= 0.0 && input->duration_discretization != RUCKIG_DURATION_CONTINUOUS) {
        return RUCKIG_ERROR_INVALID_INPUT;
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

        if (isnan(j_max) || j_max < 0.0 || isnan(a_max) || a_max < 0.0 || isnan(a_min) || a_min > 0.0) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
        if (isnan(a0) || isnan(af) || isnan(v0) || isnan(vf)) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
        if (check_current_state_within_limits && (a0 > a_max || a0 < a_min)) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
        if (check_target_state_within_limits && (af > a_max || af < a_min)) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }

        if (control_interface == RUCKIG_CONTROL_POSITION) {
            const double p0 = input->current_position[dof];
            const double pf = input->target_position[dof];
            const double v_max = input->max_velocity[dof];
            const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];

            if (isnan(p0) || isnan(pf) || isnan(v_max) || v_max < 0.0 || isnan(v_min) || v_min > 0.0) {
                return RUCKIG_ERROR_INVALID_INPUT;
            }
            if (check_current_state_within_limits && (v0 > v_max || v0 < v_min)) {
                return RUCKIG_ERROR_INVALID_INPUT;
            }
            if (check_target_state_within_limits && (vf > v_max || vf < v_min)) {
                return RUCKIG_ERROR_INVALID_INPUT;
            }
            if (check_current_state_within_limits) {
                if (a0 > 0.0 && j_max > 0.0 && v_at_a_zero(v0, a0, j_max) > v_max) {
                    return RUCKIG_ERROR_INVALID_INPUT;
                }
                if (a0 < 0.0 && j_max > 0.0 && v_at_a_zero(v0, a0, -j_max) < v_min) {
                    return RUCKIG_ERROR_INVALID_INPUT;
                }
            }
            if (check_target_state_within_limits) {
                if (af < 0.0 && j_max > 0.0 && v_at_a_zero(vf, af, j_max) > v_max) {
                    return RUCKIG_ERROR_INVALID_INPUT;
                }
                if (af > 0.0 && j_max > 0.0 && v_at_a_zero(vf, af, -j_max) < v_min) {
                    return RUCKIG_ERROR_INVALID_INPUT;
                }
            }
        }
    }

    return RUCKIG_WORKING;
}

ruckig_result_t ruckig_input_copy_state(const ruckig_input_t* src, ruckig_input_t* dst) {
    const size_t n = src && dst ? src->dofs : 0;
    if (!src || !dst || src->dofs != dst->dofs) {
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

bool ruckig_input_equals(const ruckig_input_t* lhs, const ruckig_input_t* rhs) {
    const size_t n = lhs && rhs ? lhs->dofs : 0;
    if (!lhs || !rhs || lhs->dofs != rhs->dofs) {
        return false;
    }

    return lhs->control_interface == rhs->control_interface
        && lhs->synchronization == rhs->synchronization
        && lhs->duration_discretization == rhs->duration_discretization
        && lhs->has_min_velocity == rhs->has_min_velocity
        && lhs->has_min_acceleration == rhs->has_min_acceleration
        && lhs->has_minimum_duration == rhs->has_minimum_duration
        && lhs->has_per_dof_control_interface == rhs->has_per_dof_control_interface
        && lhs->has_per_dof_synchronization == rhs->has_per_dof_synchronization
        && (!lhs->has_minimum_duration || lhs->minimum_duration == rhs->minimum_duration)
        && double_arrays_equal(lhs->current_position, rhs->current_position, n)
        && double_arrays_equal(lhs->current_velocity, rhs->current_velocity, n)
        && double_arrays_equal(lhs->current_acceleration, rhs->current_acceleration, n)
        && double_arrays_equal(lhs->target_position, rhs->target_position, n)
        && double_arrays_equal(lhs->target_velocity, rhs->target_velocity, n)
        && double_arrays_equal(lhs->target_acceleration, rhs->target_acceleration, n)
        && double_arrays_equal(lhs->max_velocity, rhs->max_velocity, n)
        && double_arrays_equal(lhs->max_acceleration, rhs->max_acceleration, n)
        && double_arrays_equal(lhs->max_jerk, rhs->max_jerk, n)
        && (!lhs->has_min_velocity || double_arrays_equal(lhs->min_velocity, rhs->min_velocity, n))
        && (!lhs->has_min_acceleration || double_arrays_equal(lhs->min_acceleration, rhs->min_acceleration, n))
        && (!lhs->has_per_dof_control_interface || control_interface_arrays_equal(lhs->per_dof_control_interface, rhs->per_dof_control_interface, n))
        && (!lhs->has_per_dof_synchronization || synchronization_arrays_equal(lhs->per_dof_synchronization, rhs->per_dof_synchronization, n))
        && bool_arrays_equal(lhs->enabled, rhs->enabled, n);
}
