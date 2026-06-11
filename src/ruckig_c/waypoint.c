#include "ruckig_c/platform_clock.h"
#include "ruckig_c/internal.h"

#include <float.h>
#include <math.h>
#include <string.h>

typedef struct waypoint_interrupt_context {
    bool enabled;
    bool interrupted;
    uint64_t start_us;
    double duration_us;
} waypoint_interrupt_context_t;

static waypoint_interrupt_context_t waypoint_interrupt_context_start(const ruckig_input_t* input) {
    waypoint_interrupt_context_t context;
    context.enabled = input && input->has_interrupt_calculation_duration;
    context.interrupted = false;
    context.start_us = context.enabled ? ruckig_platform_monotonic_time_us() : 0u;
    context.duration_us = context.enabled ? input->interrupt_calculation_duration : 0.0;
    return context;
}

static bool waypoint_interrupt_check(waypoint_interrupt_context_t* context) {
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

static bool waypoint_double_arrays_equal(const double* lhs, const double* rhs, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static bool waypoint_bool_arrays_equal(const bool* lhs, const bool* rhs, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static bool waypoint_control_interface_arrays_equal(
    const ruckig_control_interface_t* lhs,
    const ruckig_control_interface_t* rhs,
    size_t count
) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static bool waypoint_synchronization_arrays_equal(
    const ruckig_synchronization_t* lhs,
    const ruckig_synchronization_t* rhs,
    size_t count
) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static bool waypoint_planning_identity_equals(const ruckig_input_t* lhs, const ruckig_input_t* rhs) {
    const size_t n = lhs && rhs ? lhs->dofs : 0;
    const size_t waypoint_values = lhs && rhs ? lhs->waypoint_count * lhs->dofs : 0;
    const size_t section_values = lhs && rhs ? (lhs->waypoint_count + 1) * lhs->dofs : 0;
    const size_t section_count = lhs && rhs ? lhs->waypoint_count + 1 : 0;
    if (!lhs || !rhs || lhs->dofs != rhs->dofs) {
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
        && (!lhs->has_minimum_duration || lhs->minimum_duration == rhs->minimum_duration)
        && waypoint_double_arrays_equal(lhs->target_position, rhs->target_position, n)
        && waypoint_double_arrays_equal(lhs->target_velocity, rhs->target_velocity, n)
        && waypoint_double_arrays_equal(lhs->target_acceleration, rhs->target_acceleration, n)
        && waypoint_double_arrays_equal(lhs->max_velocity, rhs->max_velocity, n)
        && waypoint_double_arrays_equal(lhs->max_acceleration, rhs->max_acceleration, n)
        && waypoint_double_arrays_equal(lhs->max_jerk, rhs->max_jerk, n)
        && waypoint_double_arrays_equal(lhs->max_position, rhs->max_position, n)
        && waypoint_double_arrays_equal(lhs->min_position, rhs->min_position, n)
        && waypoint_bool_arrays_equal(lhs->enabled, rhs->enabled, n)
        && (waypoint_values == 0 || waypoint_double_arrays_equal(lhs->intermediate_positions, rhs->intermediate_positions, waypoint_values))
        && (!lhs->has_min_velocity || waypoint_double_arrays_equal(lhs->min_velocity, rhs->min_velocity, n))
        && (!lhs->has_min_acceleration || waypoint_double_arrays_equal(lhs->min_acceleration, rhs->min_acceleration, n))
        && (!lhs->has_per_dof_control_interface || waypoint_control_interface_arrays_equal(lhs->per_dof_control_interface, rhs->per_dof_control_interface, n))
        && (!lhs->has_per_dof_synchronization || waypoint_synchronization_arrays_equal(lhs->per_dof_synchronization, rhs->per_dof_synchronization, n))
        && (!lhs->has_per_section_max_velocity || waypoint_double_arrays_equal(lhs->per_section_max_velocity, rhs->per_section_max_velocity, section_values))
        && (!lhs->has_per_section_min_velocity || waypoint_double_arrays_equal(lhs->per_section_min_velocity, rhs->per_section_min_velocity, section_values))
        && (!lhs->has_per_section_max_acceleration || waypoint_double_arrays_equal(lhs->per_section_max_acceleration, rhs->per_section_max_acceleration, section_values))
        && (!lhs->has_per_section_min_acceleration || waypoint_double_arrays_equal(lhs->per_section_min_acceleration, rhs->per_section_min_acceleration, section_values))
        && (!lhs->has_per_section_max_jerk || waypoint_double_arrays_equal(lhs->per_section_max_jerk, rhs->per_section_max_jerk, section_values))
        && (!lhs->has_per_section_max_position || waypoint_double_arrays_equal(lhs->per_section_max_position, rhs->per_section_max_position, section_values))
        && (!lhs->has_per_section_min_position || waypoint_double_arrays_equal(lhs->per_section_min_position, rhs->per_section_min_position, section_values))
        && (!lhs->has_per_section_minimum_duration || waypoint_double_arrays_equal(lhs->per_section_minimum_duration, rhs->per_section_minimum_duration, section_count));
}

void ruckig_waypoint_resume_clear(ruckig_t* otg) {
    if (!otg) {
        return;
    }
    otg->waypoint_engine.active = false;
    otg->waypoint_engine.complete = false;
    otg->waypoint_engine.found = false;
    otg->waypoint_engine.initial_calculation = false;
    otg->waypoint_engine.has_published_candidate = false;
    otg->waypoint_engine.phase = RUCKIG_WAYPOINT_ENGINE_PHASE_IDLE;
    otg->waypoint_engine.best_duration = DBL_MAX;
    otg->waypoint_engine.baseline_duration = DBL_MAX;
    otg->waypoint_engine.published_duration = DBL_MAX;
    otg->waypoint_engine.refine_pass = 0;
    otg->waypoint_engine.refine_waypoint = 0;
    otg->waypoint_engine.refine_dof = 0;
    otg->waypoint_engine.refine_component = 0;
    otg->waypoint_engine.refine_attempt = 0;
    otg->waypoint_engine.refine_original = 0.0;
    otg->waypoint_engine.refine_original_valid = false;
    otg->waypoint_engine.refine_improved = false;
    otg->waypoint_engine.branch_scale = 0.0;
    otg->waypoint_engine.branch_iteration = 0;
    otg->waypoint_engine.branch_count = 0;
    otg->waypoint_engine.branch_index = 0;
    otg->waypoint_engine.branch_queue_valid = false;
    otg->waypoint_engine.branch_improved_any = false;
}

bool ruckig_waypoint_resume_can_continue(const ruckig_t* otg, const ruckig_input_t* input) {
    return otg && input
        && input->waypoint_count > 0
        && input->has_interrupt_calculation_duration
        && otg->waypoint_engine.active
        && !otg->waypoint_engine.complete
        && otg->waypoint_engine.identity_input
        && waypoint_planning_identity_equals(input, otg->waypoint_engine.identity_input);
}

static double point_position(const ruckig_input_t* input, size_t point, size_t dof) {
    if (point == 0) {
        return input->current_position[dof];
    }
    if (point == input->waypoint_count + 1) {
        return input->target_position[dof];
    }
    return input->intermediate_positions[(point - 1) * input->dofs + dof];
}

static double clamp_value(double value, double min_value, double max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static double section_max_velocity(const ruckig_input_t* input, size_t section, size_t dof) {
    return input->has_per_section_max_velocity
        ? input->per_section_max_velocity[section * input->dofs + dof]
        : input->max_velocity[dof];
}

static double section_min_velocity(const ruckig_input_t* input, size_t section, size_t dof, double max_velocity) {
    if (input->has_per_section_min_velocity) {
        return input->per_section_min_velocity[section * input->dofs + dof];
    }
    return input->has_min_velocity ? input->min_velocity[dof] : -max_velocity;
}

static double section_max_acceleration(const ruckig_input_t* input, size_t section, size_t dof) {
    return input->has_per_section_max_acceleration
        ? input->per_section_max_acceleration[section * input->dofs + dof]
        : input->max_acceleration[dof];
}

static double section_min_acceleration(const ruckig_input_t* input, size_t section, size_t dof, double max_acceleration) {
    if (input->has_per_section_min_acceleration) {
        return input->per_section_min_acceleration[section * input->dofs + dof];
    }
    return input->has_min_acceleration ? input->min_acceleration[dof] : -max_acceleration;
}

static double section_max_jerk(const ruckig_input_t* input, size_t section, size_t dof) {
    return input->has_per_section_max_jerk
        ? input->per_section_max_jerk[section * input->dofs + dof]
        : input->max_jerk[dof];
}

static double section_max_position(const ruckig_input_t* input, size_t section, size_t dof) {
    return input->has_per_section_max_position
        ? input->per_section_max_position[section * input->dofs + dof]
        : input->max_position[dof];
}

static double section_min_position(const ruckig_input_t* input, size_t section, size_t dof) {
    return input->has_per_section_min_position
        ? input->per_section_min_position[section * input->dofs + dof]
        : input->min_position[dof];
}

static double waypoint_velocity_min(const ruckig_input_t* input, size_t waypoint, size_t dof) {
    const double left_max = section_max_velocity(input, waypoint, dof);
    const double right_max = section_max_velocity(input, waypoint + 1, dof);
    const double left_min = section_min_velocity(input, waypoint, dof, left_max);
    const double right_min = section_min_velocity(input, waypoint + 1, dof, right_max);
    return left_min > right_min ? left_min : right_min;
}

static double waypoint_velocity_max(const ruckig_input_t* input, size_t waypoint, size_t dof) {
    const double left_max = section_max_velocity(input, waypoint, dof);
    const double right_max = section_max_velocity(input, waypoint + 1, dof);
    return left_max < right_max ? left_max : right_max;
}

static double waypoint_acceleration_min(const ruckig_input_t* input, size_t waypoint, size_t dof) {
    const double left_max = section_max_acceleration(input, waypoint, dof);
    const double right_max = section_max_acceleration(input, waypoint + 1, dof);
    const double left_min = section_min_acceleration(input, waypoint, dof, left_max);
    const double right_min = section_min_acceleration(input, waypoint + 1, dof, right_max);
    return left_min > right_min ? left_min : right_min;
}

static double waypoint_acceleration_max(const ruckig_input_t* input, size_t waypoint, size_t dof) {
    const double left_max = section_max_acceleration(input, waypoint, dof);
    const double right_max = section_max_acceleration(input, waypoint + 1, dof);
    return left_max < right_max ? left_max : right_max;
}

static void fill_zero_candidate(double* velocity, double* acceleration, size_t count) {
    if (count > 0) {
        memset(velocity, 0, sizeof(double) * count);
        memset(acceleration, 0, sizeof(double) * count);
    }
}

static void fill_finite_difference_candidate(
    const ruckig_input_t* input,
    double* velocity,
    double* acceleration,
    double scale
) {
    size_t waypoint;
    size_t dof;
    for (waypoint = 0; waypoint < input->waypoint_count; ++waypoint) {
        for (dof = 0; dof < input->dofs; ++dof) {
            const double prev = point_position(input, waypoint, dof);
            const double current = point_position(input, waypoint + 1, dof);
            const double next = point_position(input, waypoint + 2, dof);
            const double v_min = waypoint_velocity_min(input, waypoint, dof);
            const double v_max = waypoint_velocity_max(input, waypoint, dof);
            const double a_min = waypoint_acceleration_min(input, waypoint, dof);
            const double a_max = waypoint_acceleration_max(input, waypoint, dof);
            double direction = (next - prev) * 0.5;
            const size_t index = waypoint * input->dofs + dof;
            if (fabs(direction) < 2.2204460492503131e-16) {
                direction = next - current;
            }
            if (direction > 0.0) {
                velocity[index] = clamp_value(scale * v_max, v_min, v_max);
            } else if (direction < 0.0) {
                velocity[index] = clamp_value(scale * v_min, v_min, v_max);
            } else {
                velocity[index] = 0.0;
            }
            acceleration[index] = clamp_value(0.0, a_min, a_max);
        }
    }
}

static void apply_section_input(
    const ruckig_input_t* input,
    ruckig_input_t* section_input,
    size_t section,
    const double* waypoint_velocity,
    const double* waypoint_acceleration
) {
    size_t dof;
    section_input->control_interface = RUCKIG_CONTROL_POSITION;
    section_input->synchronization = input->synchronization;
    section_input->has_per_dof_control_interface = false;
    section_input->has_per_dof_synchronization = false;
    section_input->duration_discretization = RUCKIG_DURATION_CONTINUOUS;
    section_input->has_min_velocity = input->has_min_velocity || input->has_per_section_min_velocity;
    section_input->has_min_acceleration = input->has_min_acceleration || input->has_per_section_min_acceleration;
    section_input->has_minimum_duration = input->has_per_section_minimum_duration;
    section_input->minimum_duration = input->has_per_section_minimum_duration
        ? input->per_section_minimum_duration[section]
        : 0.0;
    section_input->waypoint_count = 0;

    for (dof = 0; dof < input->dofs; ++dof) {
        const size_t section_index = section * input->dofs + dof;
        const double max_velocity = section_max_velocity(input, section, dof);
        const double max_acceleration = section_max_acceleration(input, section, dof);
        section_input->current_position[dof] = point_position(input, section, dof);
        section_input->target_position[dof] = point_position(input, section + 1, dof);
        section_input->current_velocity[dof] = section == 0
            ? input->current_velocity[dof]
            : waypoint_velocity[(section - 1) * input->dofs + dof];
        section_input->target_velocity[dof] = section == input->waypoint_count
            ? input->target_velocity[dof]
            : waypoint_velocity[section * input->dofs + dof];
        section_input->current_acceleration[dof] = section == 0
            ? input->current_acceleration[dof]
            : waypoint_acceleration[(section - 1) * input->dofs + dof];
        section_input->target_acceleration[dof] = section == input->waypoint_count
            ? input->target_acceleration[dof]
            : waypoint_acceleration[section * input->dofs + dof];
        section_input->max_velocity[dof] = max_velocity;
        section_input->max_acceleration[dof] = max_acceleration;
        section_input->max_jerk[dof] = section_max_jerk(input, section, dof);
        section_input->min_velocity[dof] = section_min_velocity(input, section, dof, max_velocity);
        section_input->min_acceleration[dof] = section_min_acceleration(input, section, dof, max_acceleration);
        section_input->max_position[dof] = section_max_position(input, section, dof);
        section_input->min_position[dof] = section_min_position(input, section, dof);
        section_input->enabled[dof] = input->enabled[dof];
        section_input->per_dof_control_interface[dof] = RUCKIG_CONTROL_POSITION;
        section_input->per_dof_synchronization[dof] = RUCKIG_SYNCHRONIZATION_TIME;
        (void)section_index;
    }
}

static bool section_respects_position_bounds(
    const ruckig_input_t* input,
    const ruckig_trajectory_t* section_trajectory,
    size_t section
) {
    size_t dof;
    for (dof = 0; dof < input->dofs; ++dof) {
        const ruckig_bound_t bound = ruckig_profile_get_position_extrema(&section_trajectory->profiles[dof]);
        const double max_position = section_max_position(input, section, dof);
        const double min_position = section_min_position(input, section, dof);
        if (bound.max > max_position + 1.0e-9 || bound.min < min_position - 1.0e-9) {
            return false;
        }
    }
    return true;
}

static ruckig_result_t evaluate_candidate(
    ruckig_t* otg,
    const ruckig_input_t* input,
    const double* waypoint_velocity,
    const double* waypoint_acceleration,
    double* duration
) {
    size_t section;
    double total = 0.0;
    for (section = 0; section < input->waypoint_count + 1; ++section) {
        ruckig_result_t result;
        apply_section_input(input, otg->waypoint_section_input, section, waypoint_velocity, waypoint_acceleration);
        result = ruckig_calculate_target(otg, otg->waypoint_section_input, otg->waypoint_section_trajectory);
        if (result != RUCKIG_WORKING) {
            return result;
        }
        if (!section_respects_position_bounds(input, otg->waypoint_section_trajectory, section)) {
            return RUCKIG_ERROR;
        }
        total += otg->waypoint_section_trajectory->duration;
    }
    *duration = total;
    return RUCKIG_WORKING;
}

static bool accept_if_better(
    ruckig_t* otg,
    const ruckig_input_t* input,
    const double* velocity,
    const double* acceleration,
    double* best_duration
) {
    double duration = 0.0;
    const size_t count = input->waypoint_count * input->dofs;
    ++otg->waypoint_engine.last_candidate_evaluations;
    if (evaluate_candidate(otg, input, velocity, acceleration, &duration) != RUCKIG_WORKING) {
        return false;
    }
    if (duration < *best_duration) {
        *best_duration = duration;
        memcpy(otg->waypoint_engine.best_velocity, velocity, sizeof(double) * count);
        memcpy(otg->waypoint_engine.best_acceleration, acceleration, sizeof(double) * count);
        return true;
    }
    return false;
}

static double optimistic_branch_lower_bound(double best_duration, double delta, double range) {
    double normalized;
    if (best_duration == DBL_MAX || range <= 2.2204460492503131e-16) {
        return best_duration;
    }
    normalized = fabs(delta) / range;
    if (normalized > 1.0) {
        normalized = 1.0;
    }
    return best_duration * (1.0 - 0.02 * normalized);
}

static void insert_branch(
    ruckig_waypoint_branch_t* queue,
    size_t* branch_count,
    ruckig_waypoint_branch_t branch
) {
    size_t position;
    if (fabs(branch.delta) <= 2.2204460492503131e-16 || isnan(branch.lower_bound)) {
        return;
    }
    if (*branch_count == RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY
        && branch.lower_bound >= queue[*branch_count - 1].lower_bound) {
        return;
    }

    position = *branch_count;
    if (*branch_count < RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY) {
        ++(*branch_count);
    } else {
        position = RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY - 1;
    }
    while (position > 0 && queue[position - 1].lower_bound > branch.lower_bound) {
        queue[position] = queue[position - 1];
        --position;
    }
    queue[position] = branch;
}

static size_t build_branch_queue(
    const ruckig_input_t* input,
    double best_duration,
    double scale,
    ruckig_waypoint_branch_t* queue
) {
    size_t waypoint;
    size_t dof;
    size_t branch_count = 0;

    for (waypoint = 0; waypoint < input->waypoint_count; ++waypoint) {
        for (dof = 0; dof < input->dofs; ++dof) {
            const size_t index = waypoint * input->dofs + dof;
            const double v_min = waypoint_velocity_min(input, waypoint, dof);
            const double v_max = waypoint_velocity_max(input, waypoint, dof);
            const double a_min = waypoint_acceleration_min(input, waypoint, dof);
            const double a_max = waypoint_acceleration_max(input, waypoint, dof);
            const double v_step = scale * (v_max - v_min);
            const double a_step = scale * (a_max - a_min);
            ruckig_waypoint_branch_t branch;

            branch.index = index;
            branch.acceleration = false;
            branch.delta = v_step;
            branch.lower_bound = optimistic_branch_lower_bound(best_duration, v_step, v_max - v_min);
            insert_branch(queue, &branch_count, branch);
            branch.delta = -v_step;
            branch.lower_bound = optimistic_branch_lower_bound(best_duration, -v_step, v_max - v_min);
            insert_branch(queue, &branch_count, branch);

            branch.acceleration = true;
            branch.delta = a_step;
            branch.lower_bound = optimistic_branch_lower_bound(best_duration, a_step, a_max - a_min);
            insert_branch(queue, &branch_count, branch);
            branch.delta = -a_step;
            branch.lower_bound = optimistic_branch_lower_bound(best_duration, -a_step, a_max - a_min);
            insert_branch(queue, &branch_count, branch);
        }
    }

    return branch_count;
}

static void waypoint_engine_update_last_diagnostics(ruckig_t* otg) {
    otg->waypoint_engine.last_baseline_duration = otg->waypoint_engine.baseline_duration;
    otg->waypoint_engine.last_best_duration = otg->waypoint_engine.found ? otg->waypoint_engine.best_duration : DBL_MAX;
    otg->waypoint_engine.last_improved_baseline =
        otg->waypoint_engine.baseline_duration != DBL_MAX
        && otg->waypoint_engine.best_duration < otg->waypoint_engine.baseline_duration - 1.0e-12;
}

static void waypoint_engine_mark_complete(ruckig_t* otg) {
    otg->waypoint_engine.phase = RUCKIG_WAYPOINT_ENGINE_PHASE_COMPLETE;
    otg->waypoint_engine.complete = true;
    otg->waypoint_engine.active = false;
}

static ruckig_result_t waypoint_engine_start(
    ruckig_t* otg,
    const ruckig_input_t* input,
    bool initial_calculation
) {
    if (!otg || !input || input->waypoint_count == 0
        || input->waypoint_count > otg->max_number_of_waypoints
        || !otg->waypoint_section_input || !otg->waypoint_section_trajectory
        || !otg->waypoint_engine.identity_input || !otg->waypoint_engine.branch_queue) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (ruckig_validate_input(otg, input, false, true) != RUCKIG_WORKING) {
        ruckig_waypoint_resume_clear(otg);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (ruckig_input_copy_state(input, otg->waypoint_engine.identity_input) != RUCKIG_WORKING) {
        ruckig_waypoint_resume_clear(otg);
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    ruckig_waypoint_resume_clear(otg);
    otg->waypoint_engine.active = true;
    otg->waypoint_engine.complete = false;
    otg->waypoint_engine.found = false;
    otg->waypoint_engine.initial_calculation = initial_calculation;
    otg->waypoint_engine.has_published_candidate = false;
    otg->waypoint_engine.phase = RUCKIG_WAYPOINT_ENGINE_PHASE_BASELINE;
    otg->waypoint_engine.best_duration = DBL_MAX;
    otg->waypoint_engine.baseline_duration = DBL_MAX;
    otg->waypoint_engine.published_duration = DBL_MAX;
    otg->waypoint_engine.last_baseline_duration = DBL_MAX;
    otg->waypoint_engine.last_best_duration = DBL_MAX;
    otg->waypoint_engine.last_best_lower_bound = DBL_MAX;
    otg->waypoint_engine.last_candidate_evaluations = 0;
    otg->waypoint_engine.last_improved_baseline = false;
    return RUCKIG_WORKING;
}

static bool waypoint_engine_accept_if_better(
    ruckig_t* otg,
    const ruckig_input_t* input,
    const double* velocity,
    const double* acceleration
) {
    const bool accepted = accept_if_better(
        otg,
        input,
        velocity,
        acceleration,
        &otg->waypoint_engine.best_duration
    );
    if (accepted) {
        otg->waypoint_engine.found = true;
        otg->waypoint_engine.has_published_candidate = true;
    }
    return accepted;
}

static void waypoint_engine_advance_refine_index(ruckig_t* otg, const ruckig_input_t* input) {
    const size_t count = input->waypoint_count * input->dofs;
    memcpy(otg->waypoint_engine.candidate_velocity, otg->waypoint_engine.best_velocity, sizeof(double) * count);
    memcpy(otg->waypoint_engine.candidate_acceleration, otg->waypoint_engine.best_acceleration, sizeof(double) * count);

    otg->waypoint_engine.refine_component = 0;
    otg->waypoint_engine.refine_attempt = 0;
    otg->waypoint_engine.refine_original_valid = false;
    ++otg->waypoint_engine.refine_dof;
    if (otg->waypoint_engine.refine_dof >= input->dofs) {
        otg->waypoint_engine.refine_dof = 0;
        ++otg->waypoint_engine.refine_waypoint;
        if (otg->waypoint_engine.refine_waypoint >= input->waypoint_count) {
            otg->waypoint_engine.refine_waypoint = 0;
            ++otg->waypoint_engine.refine_pass;
            if (otg->waypoint_engine.refine_pass >= 2) {
                otg->waypoint_engine.phase = RUCKIG_WAYPOINT_ENGINE_PHASE_BRANCH_INIT;
            }
        }
    }
}

static ruckig_result_t waypoint_engine_step_refine(
    ruckig_t* otg,
    const ruckig_input_t* input,
    bool* candidate_evaluated
) {
    const size_t waypoint = otg->waypoint_engine.refine_waypoint;
    const size_t dof = otg->waypoint_engine.refine_dof;
    const size_t index = waypoint * input->dofs + dof;
    const double scale = otg->waypoint_engine.refine_pass == 0 ? 0.25 : 0.10;
    const double v_min = waypoint_velocity_min(input, waypoint, dof);
    const double v_max = waypoint_velocity_max(input, waypoint, dof);
    const double a_min = waypoint_acceleration_min(input, waypoint, dof);
    const double a_max = waypoint_acceleration_max(input, waypoint, dof);
    const double v_step = scale * (v_max - v_min);
    const double a_step = scale * (a_max - a_min);
    bool accepted;

    *candidate_evaluated = true;

    if (otg->waypoint_engine.refine_component == 0) {
        if (!otg->waypoint_engine.refine_original_valid) {
            otg->waypoint_engine.refine_original = otg->waypoint_engine.candidate_velocity[index];
            otg->waypoint_engine.refine_original_valid = true;
        }
        if (otg->waypoint_engine.refine_attempt == 0) {
            otg->waypoint_engine.candidate_velocity[index] =
                clamp_value(otg->waypoint_engine.refine_original + v_step, v_min, v_max);
            accepted = waypoint_engine_accept_if_better(
                otg,
                input,
                otg->waypoint_engine.candidate_velocity,
                otg->waypoint_engine.candidate_acceleration
            );
            if (accepted) {
                otg->waypoint_engine.refine_improved = true;
                otg->waypoint_engine.refine_original = otg->waypoint_engine.candidate_velocity[index];
                otg->waypoint_engine.refine_component = 1;
                otg->waypoint_engine.refine_attempt = 0;
                otg->waypoint_engine.refine_original_valid = false;
            } else {
                otg->waypoint_engine.refine_attempt = 1;
            }
            return RUCKIG_WORKING;
        }

        otg->waypoint_engine.candidate_velocity[index] =
            clamp_value(otg->waypoint_engine.refine_original - v_step, v_min, v_max);
        accepted = waypoint_engine_accept_if_better(
            otg,
            input,
            otg->waypoint_engine.candidate_velocity,
            otg->waypoint_engine.candidate_acceleration
        );
        if (accepted) {
            otg->waypoint_engine.refine_improved = true;
        } else {
            otg->waypoint_engine.candidate_velocity[index] = otg->waypoint_engine.refine_original;
        }
        otg->waypoint_engine.refine_component = 1;
        otg->waypoint_engine.refine_attempt = 0;
        otg->waypoint_engine.refine_original_valid = false;
        return RUCKIG_WORKING;
    }

    if (!otg->waypoint_engine.refine_original_valid) {
        otg->waypoint_engine.refine_original = otg->waypoint_engine.candidate_acceleration[index];
        otg->waypoint_engine.refine_original_valid = true;
    }
    if (otg->waypoint_engine.refine_attempt == 0) {
        otg->waypoint_engine.candidate_acceleration[index] =
            clamp_value(otg->waypoint_engine.refine_original + a_step, a_min, a_max);
        accepted = waypoint_engine_accept_if_better(
            otg,
            input,
            otg->waypoint_engine.candidate_velocity,
            otg->waypoint_engine.candidate_acceleration
        );
        if (accepted) {
            otg->waypoint_engine.refine_improved = true;
            waypoint_engine_advance_refine_index(otg, input);
        } else {
            otg->waypoint_engine.refine_attempt = 1;
        }
        return RUCKIG_WORKING;
    }

    otg->waypoint_engine.candidate_acceleration[index] =
        clamp_value(otg->waypoint_engine.refine_original - a_step, a_min, a_max);
    accepted = waypoint_engine_accept_if_better(
        otg,
        input,
        otg->waypoint_engine.candidate_velocity,
        otg->waypoint_engine.candidate_acceleration
    );
    if (accepted) {
        otg->waypoint_engine.refine_improved = true;
    } else {
        otg->waypoint_engine.candidate_acceleration[index] = otg->waypoint_engine.refine_original;
    }
    waypoint_engine_advance_refine_index(otg, input);
    return RUCKIG_WORKING;
}

static ruckig_result_t waypoint_engine_step_branch(
    ruckig_t* otg,
    const ruckig_input_t* input,
    bool* candidate_evaluated
) {
    const size_t count = input->waypoint_count * input->dofs;

    while (true) {
        if (otg->waypoint_engine.branch_iteration >= RUCKIG_WAYPOINT_BRANCH_ITERATION_BUDGET
            || otg->waypoint_engine.branch_scale < 0.025) {
            waypoint_engine_mark_complete(otg);
            *candidate_evaluated = false;
            return RUCKIG_WORKING;
        }

        if (!otg->waypoint_engine.branch_queue_valid) {
            otg->waypoint_engine.branch_count = build_branch_queue(
                input,
                otg->waypoint_engine.best_duration,
                otg->waypoint_engine.branch_scale,
                otg->waypoint_engine.branch_queue
            );
            otg->waypoint_engine.branch_index = 0;
            otg->waypoint_engine.branch_queue_valid = true;
            if (otg->waypoint_engine.branch_count > 0
                && otg->waypoint_engine.branch_queue[0].lower_bound < otg->waypoint_engine.last_best_lower_bound) {
                otg->waypoint_engine.last_best_lower_bound = otg->waypoint_engine.branch_queue[0].lower_bound;
            }
            if (otg->waypoint_engine.branch_count == 0) {
                otg->waypoint_engine.branch_scale *= 0.5;
                otg->waypoint_engine.branch_queue_valid = false;
                continue;
            }
        }

        if (otg->waypoint_engine.branch_index >= otg->waypoint_engine.branch_count) {
            otg->waypoint_engine.branch_scale *= 0.5;
            otg->waypoint_engine.branch_queue_valid = false;
            continue;
        }

        {
            const ruckig_waypoint_branch_t branch =
                otg->waypoint_engine.branch_queue[otg->waypoint_engine.branch_index];
            bool accepted;
            ++otg->waypoint_engine.branch_index;
            memcpy(otg->waypoint_engine.candidate_velocity, otg->waypoint_engine.best_velocity, sizeof(double) * count);
            memcpy(otg->waypoint_engine.candidate_acceleration, otg->waypoint_engine.best_acceleration, sizeof(double) * count);
            if (branch.acceleration) {
                const size_t waypoint = branch.index / input->dofs;
                const size_t dof = branch.index % input->dofs;
                const double a_min = waypoint_acceleration_min(input, waypoint, dof);
                const double a_max = waypoint_acceleration_max(input, waypoint, dof);
                otg->waypoint_engine.candidate_acceleration[branch.index] =
                    clamp_value(otg->waypoint_engine.candidate_acceleration[branch.index] + branch.delta, a_min, a_max);
            } else {
                const size_t waypoint = branch.index / input->dofs;
                const size_t dof = branch.index % input->dofs;
                const double v_min = waypoint_velocity_min(input, waypoint, dof);
                const double v_max = waypoint_velocity_max(input, waypoint, dof);
                otg->waypoint_engine.candidate_velocity[branch.index] =
                    clamp_value(otg->waypoint_engine.candidate_velocity[branch.index] + branch.delta, v_min, v_max);
            }
            accepted = waypoint_engine_accept_if_better(
                otg,
                input,
                otg->waypoint_engine.candidate_velocity,
                otg->waypoint_engine.candidate_acceleration
            );
            if (accepted) {
                otg->waypoint_engine.branch_improved_any = true;
                otg->waypoint_engine.branch_queue_valid = false;
            } else {
                ++otg->waypoint_engine.branch_iteration;
            }
            *candidate_evaluated = true;
            return RUCKIG_WORKING;
        }
    }
}

static ruckig_result_t waypoint_engine_step(
    ruckig_t* otg,
    const ruckig_input_t* input,
    bool* candidate_evaluated
) {
    const size_t count = input->waypoint_count * input->dofs;
    *candidate_evaluated = false;

    switch (otg->waypoint_engine.phase) {
    case RUCKIG_WAYPOINT_ENGINE_PHASE_BASELINE:
        fill_zero_candidate(otg->waypoint_engine.baseline_velocity, otg->waypoint_engine.baseline_acceleration, count);
        if (waypoint_engine_accept_if_better(
                otg,
                input,
                otg->waypoint_engine.baseline_velocity,
                otg->waypoint_engine.baseline_acceleration)) {
            otg->waypoint_engine.baseline_duration = otg->waypoint_engine.best_duration;
        }
        otg->waypoint_engine.phase = RUCKIG_WAYPOINT_ENGINE_PHASE_FINITE_DIFFERENCE_035;
        *candidate_evaluated = true;
        return RUCKIG_WORKING;

    case RUCKIG_WAYPOINT_ENGINE_PHASE_FINITE_DIFFERENCE_035:
        fill_finite_difference_candidate(input, otg->waypoint_engine.candidate_velocity, otg->waypoint_engine.candidate_acceleration, 0.35);
        (void)waypoint_engine_accept_if_better(
            otg,
            input,
            otg->waypoint_engine.candidate_velocity,
            otg->waypoint_engine.candidate_acceleration
        );
        otg->waypoint_engine.phase = RUCKIG_WAYPOINT_ENGINE_PHASE_FINITE_DIFFERENCE_070;
        *candidate_evaluated = true;
        return RUCKIG_WORKING;

    case RUCKIG_WAYPOINT_ENGINE_PHASE_FINITE_DIFFERENCE_070:
        fill_finite_difference_candidate(input, otg->waypoint_engine.candidate_velocity, otg->waypoint_engine.candidate_acceleration, 0.70);
        (void)waypoint_engine_accept_if_better(
            otg,
            input,
            otg->waypoint_engine.candidate_velocity,
            otg->waypoint_engine.candidate_acceleration
        );
        otg->waypoint_engine.phase = otg->waypoint_engine.found
            ? RUCKIG_WAYPOINT_ENGINE_PHASE_REFINE_INIT
            : RUCKIG_WAYPOINT_ENGINE_PHASE_COMPLETE;
        *candidate_evaluated = true;
        return RUCKIG_WORKING;

    case RUCKIG_WAYPOINT_ENGINE_PHASE_REFINE_INIT:
        if (!otg->waypoint_engine.found) {
            waypoint_engine_mark_complete(otg);
            return RUCKIG_WORKING;
        }
        memcpy(otg->waypoint_engine.candidate_velocity, otg->waypoint_engine.best_velocity, sizeof(double) * count);
        memcpy(otg->waypoint_engine.candidate_acceleration, otg->waypoint_engine.best_acceleration, sizeof(double) * count);
        otg->waypoint_engine.refine_pass = 0;
        otg->waypoint_engine.refine_waypoint = 0;
        otg->waypoint_engine.refine_dof = 0;
        otg->waypoint_engine.refine_component = 0;
        otg->waypoint_engine.refine_attempt = 0;
        otg->waypoint_engine.refine_original_valid = false;
        otg->waypoint_engine.refine_improved = false;
        otg->waypoint_engine.phase = RUCKIG_WAYPOINT_ENGINE_PHASE_REFINE;
        return RUCKIG_WORKING;

    case RUCKIG_WAYPOINT_ENGINE_PHASE_REFINE:
        if (otg->waypoint_engine.refine_pass >= 2) {
            otg->waypoint_engine.phase = RUCKIG_WAYPOINT_ENGINE_PHASE_BRANCH_INIT;
            return RUCKIG_WORKING;
        }
        return waypoint_engine_step_refine(otg, input, candidate_evaluated);

    case RUCKIG_WAYPOINT_ENGINE_PHASE_BRANCH_INIT:
        if (!otg->waypoint_engine.found) {
            waypoint_engine_mark_complete(otg);
            return RUCKIG_WORKING;
        }
        otg->waypoint_engine.branch_scale = 0.20;
        otg->waypoint_engine.branch_iteration = 0;
        otg->waypoint_engine.branch_count = 0;
        otg->waypoint_engine.branch_index = 0;
        otg->waypoint_engine.branch_queue_valid = false;
        otg->waypoint_engine.branch_improved_any = false;
        otg->waypoint_engine.phase = RUCKIG_WAYPOINT_ENGINE_PHASE_BRANCH;
        return RUCKIG_WORKING;

    case RUCKIG_WAYPOINT_ENGINE_PHASE_BRANCH:
        return waypoint_engine_step_branch(otg, input, candidate_evaluated);

    case RUCKIG_WAYPOINT_ENGINE_PHASE_COMPLETE:
        waypoint_engine_mark_complete(otg);
        return RUCKIG_WORKING;

    case RUCKIG_WAYPOINT_ENGINE_PHASE_IDLE:
    default:
        waypoint_engine_mark_complete(otg);
        return RUCKIG_WORKING;
    }
}

static ruckig_result_t waypoint_engine_run(
    ruckig_t* otg,
    const ruckig_input_t* input,
    waypoint_interrupt_context_t* interrupt
) {
    while (otg->waypoint_engine.active && !otg->waypoint_engine.complete) {
        bool candidate_evaluated = false;
        const ruckig_result_t result = waypoint_engine_step(otg, input, &candidate_evaluated);
        if (result != RUCKIG_WORKING) {
            waypoint_engine_update_last_diagnostics(otg);
            return result;
        }
        if (candidate_evaluated && waypoint_interrupt_check(interrupt)) {
            break;
        }
    }
    waypoint_engine_update_last_diagnostics(otg);
    return RUCKIG_WORKING;
}

static ruckig_result_t write_best_trajectory(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    size_t section;
    size_t dof;
    double cumulative = 0.0;
    trajectory->valid = false;
    trajectory->section_count = input->waypoint_count + 1;
    trajectory->duration = 0.0;
    for (dof = 0; dof < input->dofs; ++dof) {
        trajectory->independent_min_durations[dof] = 0.0;
    }

    for (section = 0; section < trajectory->section_count; ++section) {
        ruckig_result_t result;
        apply_section_input(input, otg->waypoint_section_input, section, otg->waypoint_engine.best_velocity, otg->waypoint_engine.best_acceleration);
        result = ruckig_calculate_target(otg, otg->waypoint_section_input, otg->waypoint_section_trajectory);
        if (result != RUCKIG_WORKING || !section_respects_position_bounds(input, otg->waypoint_section_trajectory, section)) {
            trajectory->section_count = 1;
            return result == RUCKIG_WORKING ? RUCKIG_ERROR : result;
        }
        for (dof = 0; dof < input->dofs; ++dof) {
            trajectory->profiles[section * input->dofs + dof] = otg->waypoint_section_trajectory->profiles[dof];
            trajectory->independent_min_durations[dof] += otg->waypoint_section_trajectory->independent_min_durations[dof];
        }
        cumulative += otg->waypoint_section_trajectory->duration;
        trajectory->cumulative_times[section] = cumulative;
    }

    trajectory->duration = cumulative;
    trajectory->valid = true;
    return RUCKIG_WORKING;
}

static ruckig_result_t copy_waypoint_trajectory(
    ruckig_trajectory_t* dst,
    const ruckig_trajectory_t* src
) {
    const size_t profile_count = src ? src->section_count * src->dofs : 0;
    if (!dst || !src || dst->dofs != src->dofs
        || dst->section_capacity < src->section_count) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    dst->section_count = src->section_count;
    dst->duration = src->duration;
    dst->valid = src->valid;
    if (profile_count > 0) {
        memcpy(dst->profiles, src->profiles, sizeof(ruckig_profile_t) * profile_count);
    }
    memcpy(dst->blocks, src->blocks, sizeof(ruckig_block_t) * src->dofs);
    memcpy(dst->independent_min_durations, src->independent_min_durations, sizeof(double) * src->dofs);
    memcpy(dst->cumulative_times, src->cumulative_times, sizeof(double) * src->section_count);
    return RUCKIG_WORKING;
}

static ruckig_result_t waypoint_engine_publish_best_transaction(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    ruckig_result_t result;
    if (!otg || !input || !trajectory || !otg->waypoint_engine.scratch_trajectory) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    result = write_best_trajectory(otg, input, otg->waypoint_engine.scratch_trajectory);
    if (result != RUCKIG_WORKING || !otg->waypoint_engine.scratch_trajectory->valid) {
        return result == RUCKIG_WORKING ? RUCKIG_ERROR : result;
    }

    return copy_waypoint_trajectory(trajectory, otg->waypoint_engine.scratch_trajectory);
}

static ruckig_result_t ruckig_calculate_waypoints_impl(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    ruckig_result_t result;

    if (!otg || !input || !trajectory || input->waypoint_count == 0
        || input->waypoint_count > otg->max_number_of_waypoints
        || input->waypoint_count > trajectory->max_number_of_waypoints) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    trajectory->valid = false;

    result = waypoint_engine_start(otg, input, false);
    if (result != RUCKIG_WORKING) {
        ruckig_waypoint_resume_clear(otg);
        return result;
    }
    result = waypoint_engine_run(otg, input, NULL);
    if (result != RUCKIG_WORKING) {
        ruckig_waypoint_resume_clear(otg);
        return result;
    }
    if (!otg->waypoint_engine.found) {
        ruckig_waypoint_resume_clear(otg);
        return RUCKIG_ERROR;
    }

    result = waypoint_engine_publish_best_transaction(otg, input, trajectory);
    ruckig_waypoint_resume_clear(otg);
    return result;
}

ruckig_result_t ruckig_calculate_waypoints(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    return ruckig_calculate_waypoints_impl(otg, input, trajectory);
}

ruckig_result_t ruckig_calculate_waypoints_interruptible(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    bool* was_interrupted
) {
    waypoint_interrupt_context_t interrupt;
    ruckig_result_t result;
    if (was_interrupted) {
        *was_interrupted = false;
    }
    if (!otg || !input || !trajectory || input->waypoint_count == 0
        || input->waypoint_count > trajectory->max_number_of_waypoints) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    trajectory->valid = false;
    interrupt = waypoint_interrupt_context_start(input);
    result = waypoint_engine_start(otg, input, true);
    if (result != RUCKIG_WORKING) {
        if (was_interrupted) {
            *was_interrupted = interrupt.interrupted;
        }
        return result;
    }
    result = waypoint_engine_run(otg, input, &interrupt);
    if (was_interrupted) {
        *was_interrupted = interrupt.interrupted;
    }
    if (result != RUCKIG_WORKING) {
        ruckig_waypoint_resume_clear(otg);
        return result;
    }
    if (otg->waypoint_engine.found) {
        result = waypoint_engine_publish_best_transaction(otg, input, trajectory);
        if (result != RUCKIG_WORKING) {
            ruckig_waypoint_resume_clear(otg);
            return result;
        }
        otg->waypoint_engine.published_duration = trajectory->duration;
        if (!interrupt.interrupted || otg->waypoint_engine.complete) {
            waypoint_engine_mark_complete(otg);
        }
        return RUCKIG_WORKING;
    }

    ruckig_waypoint_resume_clear(otg);
    return interrupt.interrupted ? RUCKIG_ERROR_EXECUTION_TIME_CALCULATION : RUCKIG_ERROR;
}

ruckig_result_t ruckig_waypoint_resume_continue(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    double incumbent_remaining_duration,
    bool* was_interrupted,
    bool* published
) {
    waypoint_interrupt_context_t interrupt;
    ruckig_result_t result;
    if (was_interrupted) {
        *was_interrupted = false;
    }
    if (published) {
        *published = false;
    }
    if (!otg || !input || !trajectory) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!ruckig_waypoint_resume_can_continue(otg, input)) {
        return RUCKIG_WORKING;
    }

    interrupt = waypoint_interrupt_context_start(input);
    otg->waypoint_engine.last_candidate_evaluations = 0;
    otg->waypoint_engine.initial_calculation = false;
    otg->waypoint_engine.has_published_candidate = false;
    if (incumbent_remaining_duration > 0.0
        && incumbent_remaining_duration < otg->waypoint_engine.best_duration) {
        otg->waypoint_engine.best_duration = incumbent_remaining_duration;
    }

    result = waypoint_engine_run(otg, input, &interrupt);
    if (was_interrupted) {
        *was_interrupted = interrupt.interrupted;
    }
    if (result != RUCKIG_WORKING) {
        ruckig_waypoint_resume_clear(otg);
        return result;
    }

    if (otg->waypoint_engine.has_published_candidate
        && otg->waypoint_engine.best_duration < incumbent_remaining_duration - 1.0e-12) {
        result = waypoint_engine_publish_best_transaction(otg, input, trajectory);
        if (result != RUCKIG_WORKING) {
            ruckig_waypoint_resume_clear(otg);
            return result == RUCKIG_ERROR ? RUCKIG_WORKING : result;
        }
        otg->waypoint_engine.published_duration = trajectory->duration;
        if (published) {
            *published = true;
        }
    }
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_filter_intermediate_positions(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    const double* threshold_distance,
    size_t threshold_count,
    double* filtered_positions,
    size_t capacity,
    size_t* written_waypoints
) {
    size_t output_count = 0;
    size_t start = 0;
    size_t end;
    if (!otg || !input || !threshold_distance || !written_waypoints
        || otg->dofs != input->dofs || threshold_count != input->dofs) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    *written_waypoints = 0;
    if (input->waypoint_count == 0) {
        return RUCKIG_WORKING;
    }
    if (!filtered_positions || capacity < input->waypoint_count * input->dofs) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    for (end = 2; end < input->waypoint_count + 2; ++end) {
        bool are_all_below = true;
        size_t current;
        for (current = start + 1; current < end; ++current) {
            double t_start_max = 0.0;
            double t_end_min = 1.0;
            size_t dof;
            for (dof = 0; dof < input->dofs; ++dof) {
                const double pos_start = point_position(input, start, dof);
                const double pos_end = point_position(input, end, dof);
                const double pos_current = point_position(input, current, dof);
                const double delta = pos_end - pos_start;
                double t_start;
                double t_end_value;
                if (threshold_distance[dof] < 0.0 || isnan(threshold_distance[dof])) {
                    return RUCKIG_ERROR_INVALID_INPUT;
                }
                if (fabs(delta) < 2.2204460492503131e-16) {
                    if (fabs(pos_current - pos_start) > threshold_distance[dof]) {
                        are_all_below = false;
                        break;
                    }
                    continue;
                }
                {
                    const double h0 = (pos_current - pos_start) / delta;
                    const double margin = threshold_distance[dof] / fabs(delta);
                    t_start = h0 - margin;
                    t_end_value = h0 + margin;
                }
                if (t_start > t_start_max) {
                    t_start_max = t_start;
                }
                if (t_end_value < t_end_min) {
                    t_end_min = t_end_value;
                }
                if (t_start_max > t_end_min) {
                    are_all_below = false;
                    break;
                }
            }
            if (!are_all_below) {
                break;
            }
        }
        if (!are_all_below) {
            size_t dof;
            const size_t active_waypoint = end - 2;
            for (dof = 0; dof < input->dofs; ++dof) {
                filtered_positions[output_count * input->dofs + dof] = point_position(input, active_waypoint + 1, dof);
            }
            ++output_count;
            start = end - 1;
        }
    }

    *written_waypoints = output_count;
    return RUCKIG_WORKING;
}
