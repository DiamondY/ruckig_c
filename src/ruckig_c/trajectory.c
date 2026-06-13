#include "ruckig_c/internal.h"

#include "ruckig_c/utils.h"

#include <math.h>

static ruckig_result_t ruckig_trajectory_create_impl(
    ruckig_trajectory_t** trajectory,
    size_t dofs,
    size_t max_number_of_waypoints
) {
    ruckig_trajectory_t* value;
    const size_t section_capacity = max_number_of_waypoints + 1;
    if (!trajectory || dofs == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    *trajectory = NULL;
    value = (ruckig_trajectory_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }

    value->dofs = dofs;
    value->max_number_of_waypoints = max_number_of_waypoints;
    value->section_capacity = section_capacity;
    value->section_count = 1;
    value->profiles = (ruckig_profile_t*)ruckig_calloc(section_capacity * dofs, sizeof(ruckig_profile_t));
    value->blocks = (ruckig_block_t*)ruckig_calloc(dofs, sizeof(ruckig_block_t));
    value->independent_min_durations = ruckig_allocate_double_vector(dofs);
    value->cumulative_times = ruckig_allocate_double_vector(section_capacity);
    if (!value->profiles || !value->blocks || !value->independent_min_durations || !value->cumulative_times) {
        ruckig_trajectory_destroy(value);
        return RUCKIG_ERROR;
    }

    *trajectory = value;
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_trajectory_create(ruckig_trajectory_t** trajectory, size_t dofs) {
    return ruckig_trajectory_create_impl(trajectory, dofs, 0);
}

RUCKIG_C_API ruckig_result_t ruckig_trajectory_create_with_waypoints(
    ruckig_trajectory_t** trajectory,
    size_t dofs,
    size_t max_number_of_waypoints
) {
    return ruckig_trajectory_create_impl(trajectory, dofs, max_number_of_waypoints);
}

RUCKIG_C_API void ruckig_trajectory_destroy(ruckig_trajectory_t* trajectory) {
    if (!trajectory) {
        return;
    }
    ruckig_free(trajectory->profiles);
    ruckig_free(trajectory->blocks);
    ruckig_free(trajectory->independent_min_durations);
    ruckig_free(trajectory->cumulative_times);
    ruckig_free(trajectory);
}

RUCKIG_C_API size_t ruckig_trajectory_get_dof_count(const ruckig_trajectory_t* trajectory) {
    return trajectory ? trajectory->dofs : 0;
}

RUCKIG_C_API double ruckig_trajectory_get_duration(const ruckig_trajectory_t* trajectory) {
    return trajectory ? trajectory->duration : 0.0;
}

RUCKIG_C_API size_t ruckig_trajectory_get_section_count(const ruckig_trajectory_t* trajectory) {
    return trajectory && trajectory->valid ? trajectory->section_count : 0;
}

RUCKIG_C_API size_t ruckig_trajectory_get_intermediate_duration_count(const ruckig_trajectory_t* trajectory) {
    return trajectory && trajectory->valid && trajectory->section_count > 0 ? trajectory->section_count - 1 : 0;
}

RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_intermediate_durations(
    const ruckig_trajectory_t* trajectory,
    double* durations,
    size_t duration_count
) {
    size_t i;
    const size_t count = ruckig_trajectory_get_intermediate_duration_count(trajectory);
    if (!trajectory || !durations || duration_count < count || !trajectory->valid) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    for (i = 0; i < count; ++i) {
        durations[i] = trajectory->cumulative_times[i];
    }
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_independent_min_durations(
    const ruckig_trajectory_t* trajectory,
    double* durations,
    size_t duration_count
) {
    size_t i;
    if (!trajectory || !durations || duration_count < trajectory->dofs || !trajectory->valid) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    for (i = 0; i < trajectory->dofs; ++i) {
        durations[i] = trajectory->independent_min_durations[i];
    }
    return RUCKIG_WORKING;
}

static void profile_state_at_time(
    const ruckig_profile_t* profile,
    double time,
    double* position,
    double* velocity,
    double* acceleration,
    double* jerk
) {
    double t_diff = time;
    size_t index = 0;

    if (profile->brake.duration > 0.0) {
        if (t_diff < profile->brake.duration) {
            size_t brake_index = t_diff < profile->brake.t[0] ? 0 : 1;
            if (brake_index > 0) {
                t_diff -= profile->brake.t[brake_index - 1];
            }
            ruckig_integrate(t_diff, profile->brake.p[brake_index], profile->brake.v[brake_index], profile->brake.a[brake_index], profile->brake.j[brake_index], position, velocity, acceleration);
            if (jerk) {
                *jerk = profile->brake.j[brake_index];
            }
            return;
        }
        t_diff -= profile->brake.duration;
    }

    if (t_diff >= profile->t_sum[6]) {
        ruckig_integrate(t_diff - profile->t_sum[6], profile->p[7], profile->v[7], profile->a[7], 0.0, position, velocity, acceleration);
        if (jerk) {
            *jerk = 0.0;
        }
        return;
    }

    while (index < 7 && profile->t_sum[index] <= t_diff) {
        ++index;
    }
    if (index > 0) {
        t_diff -= profile->t_sum[index - 1];
    }

    ruckig_integrate(t_diff, profile->p[index], profile->v[index], profile->a[index], profile->j[index], position, velocity, acceleration);
    if (jerk) {
        *jerk = profile->j[index];
    }
}

RUCKIG_C_API ruckig_result_t ruckig_trajectory_at_time(
    const ruckig_trajectory_t* trajectory,
    double time,
    double* position,
    double* velocity,
    double* acceleration,
    double* jerk,
    size_t* section
) {
    size_t i;
    size_t selected_section = 0;
    double section_start = 0.0;
    double local_time;
    if (!trajectory || !position || time < 0.0 || isnan(time) || !trajectory->valid) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    if (trajectory->section_count == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (time >= trajectory->duration) {
        selected_section = trajectory->section_count - 1;
        section_start = selected_section > 0 ? trajectory->cumulative_times[selected_section - 1] : 0.0;
    } else {
        for (selected_section = 0; selected_section < trajectory->section_count; ++selected_section) {
            if (time < trajectory->cumulative_times[selected_section]) {
                break;
            }
            section_start = trajectory->cumulative_times[selected_section];
        }
        if (selected_section >= trajectory->section_count) {
            selected_section = trajectory->section_count - 1;
            section_start = selected_section > 0 ? trajectory->cumulative_times[selected_section - 1] : 0.0;
        }
    }
    local_time = time - section_start;

    for (i = 0; i < trajectory->dofs; ++i) {
        double local_velocity;
        double local_acceleration;
        profile_state_at_time(
            &trajectory->profiles[selected_section * trajectory->dofs + i],
            local_time,
            &position[i],
            velocity ? &velocity[i] : &local_velocity,
            acceleration ? &acceleration[i] : &local_acceleration,
            jerk ? &jerk[i] : NULL
        );
    }
    if (section) {
        *section = time >= trajectory->duration ? trajectory->section_count : selected_section;
    }
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_position_extrema(
    const ruckig_trajectory_t* trajectory,
    ruckig_position_extrema_t* extrema,
    size_t extrema_count
) {
    size_t i;
    if (!trajectory || !extrema || extrema_count < trajectory->dofs || !trajectory->valid) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    for (i = 0; i < trajectory->dofs; ++i) {
        size_t section;
        bool initialized = false;
        extrema[i].min_position = INFINITY;
        extrema[i].max_position = -INFINITY;
        extrema[i].time_min = 0.0;
        extrema[i].time_max = 0.0;
        for (section = 0; section < trajectory->section_count; ++section) {
            const double offset = section > 0 ? trajectory->cumulative_times[section - 1] : 0.0;
            const ruckig_bound_t bound = ruckig_profile_get_position_extrema(&trajectory->profiles[section * trajectory->dofs + i]);
            if (!initialized || bound.min < extrema[i].min_position) {
                extrema[i].min_position = bound.min;
                extrema[i].time_min = offset + bound.t_min;
            }
            if (!initialized || bound.max > extrema[i].max_position) {
                extrema[i].max_position = bound.max;
                extrema[i].time_max = offset + bound.t_max;
            }
            initialized = true;
        }
    }
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_first_time_at_position(
    const ruckig_trajectory_t* trajectory,
    size_t dof,
    double position,
    double time_after,
    double* time,
    bool* found
) {
    size_t section;
    if (!trajectory || dof >= trajectory->dofs || time_after < 0.0 || !time || !found || !trajectory->valid) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    for (section = 0; section < trajectory->section_count; ++section) {
        double local_time = 0.0;
        const double offset = section > 0 ? trajectory->cumulative_times[section - 1] : 0.0;
        const double local_time_after = time_after > offset ? time_after - offset : 0.0;
        if (ruckig_profile_get_first_state_at_position(
                &trajectory->profiles[section * trajectory->dofs + dof],
                position,
                &local_time,
                local_time_after
            )) {
            *time = offset + local_time;
            *found = true;
            return RUCKIG_WORKING;
        }
    }
    *found = false;
    *time = 0.0;
    return RUCKIG_WORKING;
}
