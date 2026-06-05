#include "ruckig_c/internal.h"

#include <float.h>
#include <math.h>
#include <string.h>

#define RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY 64u
#define RUCKIG_WAYPOINT_BRANCH_ITERATION_BUDGET 256u

typedef struct waypoint_branch {
    size_t index;
    bool acceleration;
    double delta;
    double lower_bound;
} waypoint_branch_t;

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
    ++otg->waypoint_last_candidate_evaluations;
    if (evaluate_candidate(otg, input, velocity, acceleration, &duration) != RUCKIG_WORKING) {
        return false;
    }
    if (duration < *best_duration) {
        *best_duration = duration;
        memcpy(otg->waypoint_best_velocity, velocity, sizeof(double) * count);
        memcpy(otg->waypoint_best_acceleration, acceleration, sizeof(double) * count);
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
    waypoint_branch_t* queue,
    size_t* branch_count,
    waypoint_branch_t branch
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
    waypoint_branch_t* queue
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
            waypoint_branch_t branch;

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

static bool explore_branch_queue(
    ruckig_t* otg,
    const ruckig_input_t* input,
    double* best_duration
) {
    bool improved_any = false;
    bool pass_improved = true;
    size_t iteration = 0;
    const size_t count = input->waypoint_count * input->dofs;
    double scale = 0.20;

    while (pass_improved && iteration < RUCKIG_WAYPOINT_BRANCH_ITERATION_BUDGET && scale >= 0.025) {
        waypoint_branch_t queue[RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY];
        size_t branch_count;
        size_t branch_index;
        pass_improved = false;
        branch_count = build_branch_queue(input, *best_duration, scale, queue);
        if (branch_count > 0 && queue[0].lower_bound < otg->waypoint_last_best_lower_bound) {
            otg->waypoint_last_best_lower_bound = queue[0].lower_bound;
        }

        for (branch_index = 0;
             branch_index < branch_count && iteration < RUCKIG_WAYPOINT_BRANCH_ITERATION_BUDGET;
             ++branch_index, ++iteration) {
            const waypoint_branch_t branch = queue[branch_index];
            memcpy(otg->waypoint_candidate_velocity, otg->waypoint_best_velocity, sizeof(double) * count);
            memcpy(otg->waypoint_candidate_acceleration, otg->waypoint_best_acceleration, sizeof(double) * count);
            if (branch.acceleration) {
                const size_t waypoint = branch.index / input->dofs;
                const size_t dof = branch.index % input->dofs;
                const double a_min = waypoint_acceleration_min(input, waypoint, dof);
                const double a_max = waypoint_acceleration_max(input, waypoint, dof);
                otg->waypoint_candidate_acceleration[branch.index] =
                    clamp_value(otg->waypoint_candidate_acceleration[branch.index] + branch.delta, a_min, a_max);
            } else {
                const size_t waypoint = branch.index / input->dofs;
                const size_t dof = branch.index % input->dofs;
                const double v_min = waypoint_velocity_min(input, waypoint, dof);
                const double v_max = waypoint_velocity_max(input, waypoint, dof);
                otg->waypoint_candidate_velocity[branch.index] =
                    clamp_value(otg->waypoint_candidate_velocity[branch.index] + branch.delta, v_min, v_max);
            }
            if (accept_if_better(otg, input, otg->waypoint_candidate_velocity, otg->waypoint_candidate_acceleration, best_duration)) {
                improved_any = true;
                pass_improved = true;
                break;
            }
        }

        if (!pass_improved) {
            scale *= 0.5;
            pass_improved = true;
        }
    }

    return improved_any;
}

static bool refine_candidate(
    ruckig_t* otg,
    const ruckig_input_t* input,
    double* best_duration
) {
    bool improved = false;
    size_t pass;
    const size_t count = input->waypoint_count * input->dofs;
    memcpy(otg->waypoint_candidate_velocity, otg->waypoint_best_velocity, sizeof(double) * count);
    memcpy(otg->waypoint_candidate_acceleration, otg->waypoint_best_acceleration, sizeof(double) * count);

    for (pass = 0; pass < 2; ++pass) {
        size_t waypoint;
        size_t dof;
        const double scale = pass == 0 ? 0.25 : 0.10;
        for (waypoint = 0; waypoint < input->waypoint_count; ++waypoint) {
            for (dof = 0; dof < input->dofs; ++dof) {
                const size_t index = waypoint * input->dofs + dof;
                const double v_min = waypoint_velocity_min(input, waypoint, dof);
                const double v_max = waypoint_velocity_max(input, waypoint, dof);
                const double a_min = waypoint_acceleration_min(input, waypoint, dof);
                const double a_max = waypoint_acceleration_max(input, waypoint, dof);
                const double v_step = scale * (v_max - v_min);
                const double a_step = scale * (a_max - a_min);
                double original;

                original = otg->waypoint_candidate_velocity[index];
                otg->waypoint_candidate_velocity[index] = clamp_value(original + v_step, v_min, v_max);
                if (accept_if_better(otg, input, otg->waypoint_candidate_velocity, otg->waypoint_candidate_acceleration, best_duration)) {
                    improved = true;
                    original = otg->waypoint_candidate_velocity[index];
                } else {
                    otg->waypoint_candidate_velocity[index] = clamp_value(original - v_step, v_min, v_max);
                    if (accept_if_better(otg, input, otg->waypoint_candidate_velocity, otg->waypoint_candidate_acceleration, best_duration)) {
                        improved = true;
                        original = otg->waypoint_candidate_velocity[index];
                    } else {
                        otg->waypoint_candidate_velocity[index] = original;
                    }
                }

                original = otg->waypoint_candidate_acceleration[index];
                otg->waypoint_candidate_acceleration[index] = clamp_value(original + a_step, a_min, a_max);
                if (accept_if_better(otg, input, otg->waypoint_candidate_velocity, otg->waypoint_candidate_acceleration, best_duration)) {
                    improved = true;
                } else {
                    otg->waypoint_candidate_acceleration[index] = clamp_value(original - a_step, a_min, a_max);
                    if (accept_if_better(otg, input, otg->waypoint_candidate_velocity, otg->waypoint_candidate_acceleration, best_duration)) {
                        improved = true;
                    } else {
                        otg->waypoint_candidate_acceleration[index] = original;
                    }
                }
                memcpy(otg->waypoint_candidate_velocity, otg->waypoint_best_velocity, sizeof(double) * count);
                memcpy(otg->waypoint_candidate_acceleration, otg->waypoint_best_acceleration, sizeof(double) * count);
            }
        }
    }
    return improved;
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
        apply_section_input(input, otg->waypoint_section_input, section, otg->waypoint_best_velocity, otg->waypoint_best_acceleration);
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

ruckig_result_t ruckig_calculate_waypoints(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    const size_t count = input ? input->waypoint_count * input->dofs : 0;
    double best_duration = DBL_MAX;
    bool found = false;

    if (!otg || !input || !trajectory || input->waypoint_count == 0
        || input->waypoint_count > otg->max_number_of_waypoints
        || input->waypoint_count > trajectory->max_number_of_waypoints
        || !otg->waypoint_section_input || !otg->waypoint_section_trajectory) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    trajectory->valid = false;
    otg->waypoint_last_baseline_duration = DBL_MAX;
    otg->waypoint_last_best_duration = DBL_MAX;
    otg->waypoint_last_best_lower_bound = DBL_MAX;
    otg->waypoint_last_candidate_evaluations = 0;
    otg->waypoint_last_improved_baseline = false;
    if (ruckig_validate_input(otg, input, false, true) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    fill_zero_candidate(otg->waypoint_baseline_velocity, otg->waypoint_baseline_acceleration, count);
    found = accept_if_better(otg, input, otg->waypoint_baseline_velocity, otg->waypoint_baseline_acceleration, &best_duration);
    if (found) {
        otg->waypoint_last_baseline_duration = best_duration;
    }

    fill_finite_difference_candidate(input, otg->waypoint_candidate_velocity, otg->waypoint_candidate_acceleration, 0.35);
    found = accept_if_better(otg, input, otg->waypoint_candidate_velocity, otg->waypoint_candidate_acceleration, &best_duration) || found;

    fill_finite_difference_candidate(input, otg->waypoint_candidate_velocity, otg->waypoint_candidate_acceleration, 0.70);
    found = accept_if_better(otg, input, otg->waypoint_candidate_velocity, otg->waypoint_candidate_acceleration, &best_duration) || found;

    if (found) {
        (void)refine_candidate(otg, input, &best_duration);
        (void)explore_branch_queue(otg, input, &best_duration);
        otg->waypoint_last_best_duration = best_duration;
        otg->waypoint_last_improved_baseline =
            otg->waypoint_last_baseline_duration != DBL_MAX
            && best_duration < otg->waypoint_last_baseline_duration - 1.0e-12;
        return write_best_trajectory(otg, input, trajectory);
    }

    return RUCKIG_ERROR;
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
