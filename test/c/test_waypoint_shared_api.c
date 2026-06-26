#include "test_api_internal.h"

void check_waypoint_samples(
    const ruckig_trajectory_t* trajectory,
    const double* waypoints,
    size_t waypoint_count,
    size_t dofs
) {
    double durations[4] = {0.0, 0.0, 0.0, 0.0};
    double position[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    size_t waypoint;
    size_t dof;

    CHECK_TRUE(waypoint_count <= 4);
    CHECK_TRUE(dofs <= 8);
    CHECK_EQ_INT(ruckig_trajectory_get_section_count(trajectory), waypoint_count + 1);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_duration_count(trajectory), waypoint_count);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, waypoint_count), RUCKIG_WORKING);
    for (waypoint = 0; waypoint < waypoint_count; ++waypoint) {
        size_t section = 99;
        CHECK_TRUE(durations[waypoint] > 0.0);
        CHECK_TRUE(durations[waypoint] < ruckig_trajectory_get_duration(trajectory));
        if (waypoint > 0) {
            CHECK_TRUE(durations[waypoint] > durations[waypoint - 1]);
        }
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, durations[waypoint], position, NULL, NULL, NULL, &section), RUCKIG_WORKING);
        CHECK_EQ_INT(section, waypoint + 1);
        for (dof = 0; dof < dofs; ++dof) {
            CHECK_NEAR(position[dof], waypoints[waypoint * dofs + dof], 1e-7);
        }
    }
}

void check_waypoint_section_sampled_limits(
    const ruckig_trajectory_t* trajectory,
    const double* per_section_min_velocity,
    const double* per_section_max_velocity,
    const double* per_section_min_acceleration,
    const double* per_section_max_acceleration,
    const double* per_section_max_jerk,
    const double* per_section_min_position,
    const double* per_section_max_position,
    size_t section_count,
    size_t dofs
) {
    double durations[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double position[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double velocity[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double acceleration[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double jerk[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    const double total_duration = ruckig_trajectory_get_duration(trajectory);
    size_t section;

    CHECK_TRUE(section_count <= 8);
    CHECK_TRUE(dofs <= 8);
    CHECK_EQ_INT(ruckig_trajectory_get_section_count(trajectory), section_count);
    if (section_count > 1) {
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, section_count - 1), RUCKIG_WORKING);
    }

    for (section = 0; section < section_count; ++section) {
        const double start = section == 0 ? 0.0 : durations[section - 1];
        const double end = section + 1 == section_count ? total_duration : durations[section];
        size_t sample_index;

        CHECK_TRUE(end > start);
        for (sample_index = 0; sample_index <= 6; ++sample_index) {
            const bool boundary_sample = sample_index == 0 || sample_index == 6;
            const double t = start + (end - start) * ((double)sample_index / 6.0);
            size_t sampled_section = 99;
            size_t dof;
            CHECK_EQ_INT(ruckig_trajectory_at_time(
                             trajectory,
                             t,
                             position,
                             velocity,
                             acceleration,
                             jerk,
                             &sampled_section),
                         RUCKIG_WORKING);
            if (!boundary_sample) {
                CHECK_EQ_INT(sampled_section, section);
            }
            for (dof = 0; dof < dofs; ++dof) {
                const size_t index = section * dofs + dof;
                if (per_section_min_position) {
                    CHECK_TRUE(position[dof] >= per_section_min_position[index] - 1e-7);
                }
                if (per_section_max_position) {
                    CHECK_TRUE(position[dof] <= per_section_max_position[index] + 1e-7);
                }
                if (per_section_min_velocity) {
                    CHECK_TRUE(velocity[dof] >= per_section_min_velocity[index] - 1e-7);
                }
                if (per_section_max_velocity) {
                    CHECK_TRUE(velocity[dof] <= per_section_max_velocity[index] + 1e-7);
                }
                if (per_section_min_acceleration) {
                    CHECK_TRUE(acceleration[dof] >= per_section_min_acceleration[index] - 1e-7);
                }
                if (per_section_max_acceleration) {
                    CHECK_TRUE(acceleration[dof] <= per_section_max_acceleration[index] + 1e-7);
                }
                if (!boundary_sample && per_section_max_jerk && isfinite(per_section_max_jerk[index])) {
                    CHECK_TRUE(fabs(jerk[dof]) <= per_section_max_jerk[index] + 1e-7);
                }
            }
        }
    }
}

void configure_soft_interruption_waypoint_input(ruckig_input_t* input) {
    double waypoint[1] = {1.0};
    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 1.2;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 4.0;
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 1), RUCKIG_WORKING);
}

void configure_alpha2_resume_input(ruckig_input_t* input) {
    const size_t dofs = 3;
    double waypoints[6] = {
        0.35, -0.12, 0.20,
        0.82, -0.35, 0.48
    };
    double per_section_min_velocity[9] = {
        -0.75, -0.70, -0.65,
        -0.85, -0.80, -0.75,
        -0.95, -0.90, -0.85
    };
    double per_section_max_velocity[9] = {
        0.85, 0.80, 0.75,
        0.95, 0.90, 0.85,
        1.05, 1.00, 0.95
    };
    double per_section_min_acceleration[9] = {
        -1.4, -1.3, -1.2,
        -1.5, -1.4, -1.3,
        -1.6, -1.5, -1.4
    };
    double per_section_max_acceleration[9] = {
        1.4, 1.3, 1.2,
        1.5, 1.4, 1.3,
        1.6, 1.5, 1.4
    };
    double per_section_max_jerk[9] = {
        3.6, 3.4, 3.2,
        3.8, 3.6, 3.4,
        4.0, 3.8, 3.6
    };
    double per_section_min_position[9] = {
        -0.05, -0.18, -0.05,
        0.25, -0.42, 0.12,
        0.72, -0.62, 0.40
    };
    double per_section_max_position[9] = {
        0.42, 0.05, 0.25,
        0.90, -0.05, 0.55,
        1.25, -0.30, 0.82
    };
    double per_section_minimum_duration[3] = {0.30, 0.42, 0.36};
    size_t i;

    for (i = 0; i < dofs; ++i) {
        ruckig_input_max_velocity_data(input)[i] = 1.2;
        ruckig_input_max_acceleration_data(input)[i] = 1.8;
        ruckig_input_max_jerk_data(input)[i] = 4.5;
    }
    ruckig_input_target_position_data(input)[0] = 1.12;
    ruckig_input_target_position_data(input)[1] = -0.54;
    ruckig_input_target_position_data(input)[2] = 0.72;
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 3), RUCKIG_WORKING);
}

void configure_alpha1_resume_stress_input(ruckig_input_t* input) {
    const size_t dofs = 4;
    double waypoints[12] = {
        0.25, -0.10, 0.15, -0.05,
        0.55, -0.28, 0.34, -0.18,
        0.90, -0.50, 0.58, -0.34
    };
    double per_section_min_velocity[16] = {
        -0.80, -0.75, -0.70, -0.65,
        -0.85, -0.80, -0.75, -0.70,
        -0.95, -0.90, -0.85, -0.80,
        -1.05, -1.00, -0.95, -0.90
    };
    double per_section_max_velocity[16] = {
        0.90, 0.85, 0.80, 0.75,
        0.98, 0.92, 0.86, 0.80,
        1.08, 1.00, 0.94, 0.88,
        1.18, 1.10, 1.02, 0.96
    };
    double per_section_min_acceleration[16] = {
        -1.5, -1.4, -1.3, -1.2,
        -1.6, -1.5, -1.4, -1.3,
        -1.7, -1.6, -1.5, -1.4,
        -1.8, -1.7, -1.6, -1.5
    };
    double per_section_max_acceleration[16] = {
        1.5, 1.4, 1.3, 1.2,
        1.6, 1.5, 1.4, 1.3,
        1.7, 1.6, 1.5, 1.4,
        1.8, 1.7, 1.6, 1.5
    };
    double per_section_max_jerk[16] = {
        3.8, 3.6, 3.4, 3.2,
        4.0, 3.8, 3.6, 3.4,
        4.2, 4.0, 3.8, 3.6,
        4.4, 4.2, 4.0, 3.8
    };
    double per_section_min_position[16] = {
        -0.05, -0.15, -0.05, -0.10,
        0.18, -0.33, 0.10, -0.22,
        0.48, -0.55, 0.28, -0.38,
        0.82, -0.75, 0.52, -0.55
    };
    double per_section_max_position[16] = {
        0.32, 0.03, 0.22, 0.02,
        0.62, -0.05, 0.40, -0.02,
        0.98, -0.22, 0.64, -0.14,
        1.35, -0.45, 0.90, -0.30
    };
    double per_section_minimum_duration[4] = {0.55, 0.45, 0.45, 0.40};
    size_t i;

    for (i = 0; i < dofs; ++i) {
        ruckig_input_max_velocity_data(input)[i] = 1.4;
        ruckig_input_max_acceleration_data(input)[i] = 2.0;
        ruckig_input_max_jerk_data(input)[i] = 4.8;
    }
    ruckig_input_target_position_data(input)[0] = 1.25;
    ruckig_input_target_position_data(input)[1] = -0.70;
    ruckig_input_target_position_data(input)[2] = 0.82;
    ruckig_input_target_position_data(input)[3] = -0.50;
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 4), RUCKIG_WORKING);
}

void check_alpha1_resume_stress_trajectory(const ruckig_trajectory_t* trajectory) {
    const size_t dofs = 4;
    double waypoints[12] = {
        0.25, -0.10, 0.15, -0.05,
        0.55, -0.28, 0.34, -0.18,
        0.90, -0.50, 0.58, -0.34
    };
    double per_section_min_velocity[16] = {
        -0.80, -0.75, -0.70, -0.65,
        -0.85, -0.80, -0.75, -0.70,
        -0.95, -0.90, -0.85, -0.80,
        -1.05, -1.00, -0.95, -0.90
    };
    double per_section_max_velocity[16] = {
        0.90, 0.85, 0.80, 0.75,
        0.98, 0.92, 0.86, 0.80,
        1.08, 1.00, 0.94, 0.88,
        1.18, 1.10, 1.02, 0.96
    };
    double per_section_min_acceleration[16] = {
        -1.5, -1.4, -1.3, -1.2,
        -1.6, -1.5, -1.4, -1.3,
        -1.7, -1.6, -1.5, -1.4,
        -1.8, -1.7, -1.6, -1.5
    };
    double per_section_max_acceleration[16] = {
        1.5, 1.4, 1.3, 1.2,
        1.6, 1.5, 1.4, 1.3,
        1.7, 1.6, 1.5, 1.4,
        1.8, 1.7, 1.6, 1.5
    };
    double per_section_max_jerk[16] = {
        3.8, 3.6, 3.4, 3.2,
        4.0, 3.8, 3.6, 3.4,
        4.2, 4.0, 3.8, 3.6,
        4.4, 4.2, 4.0, 3.8
    };
    double per_section_min_position[16] = {
        -0.05, -0.15, -0.05, -0.10,
        0.18, -0.33, 0.10, -0.22,
        0.48, -0.55, 0.28, -0.38,
        0.82, -0.75, 0.52, -0.55
    };
    double per_section_max_position[16] = {
        0.32, 0.03, 0.22, 0.02,
        0.62, -0.05, 0.40, -0.02,
        0.98, -0.22, 0.64, -0.14,
        1.35, -0.45, 0.90, -0.30
    };
    double per_section_minimum_duration[4] = {0.55, 0.45, 0.45, 0.40};
    double durations[3] = {0.0, 0.0, 0.0};
    const double total_duration = ruckig_trajectory_get_duration(trajectory);
    size_t section;

    CHECK_TRUE(total_duration > 0.0);
    check_waypoint_samples(trajectory, waypoints, 3, dofs);
    check_waypoint_section_sampled_limits(
        trajectory,
        per_section_min_velocity,
        per_section_max_velocity,
        per_section_min_acceleration,
        per_section_max_acceleration,
        per_section_max_jerk,
        per_section_min_position,
        per_section_max_position,
        4,
        dofs);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 3), RUCKIG_WORKING);
    for (section = 0; section < 4; ++section) {
        const double start = section == 0 ? 0.0 : durations[section - 1];
        const double end = section == 3 ? total_duration : durations[section];
        CHECK_TRUE(end > start);
        CHECK_TRUE(end - start >= per_section_minimum_duration[section] - 1e-9);
    }
}

ruckig_result_t ruckig_update_under_allocation_guard(
    ruckig_t* otg,
    ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_result_t result;
    const size_t allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    result = ruckig_update(otg, input, output);
    ruckig_allocation_forbidden_set(false);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    return result;
}
