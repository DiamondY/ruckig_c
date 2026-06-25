#include "test_api_internal.h"

static void test_waypoint_constructors_storage_and_optimizer(void) {
    ruckig_t* otg = NULL;
    ruckig_t* section_otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_input_t* section_input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_trajectory_t* section_trajectory = NULL;
    double waypoint[1] = {1.0};
    double per_section_max_velocity[2] = {1.2, 1.0};
    double per_section_min_velocity[2] = {-1.2, -1.0};
    double per_section_max_acceleration[2] = {2.0, 2.0};
    double per_section_min_acceleration[2] = {-2.0, -2.0};
    double per_section_max_jerk[2] = {4.0, 4.0};
    double per_section_max_position[2] = {1.5, 2.5};
    double per_section_min_position[2] = {-0.5, 0.5};
    double per_section_minimum_duration[2] = {0.0, 0.0};
    double waypoint_readback[1] = {0.0};
    double per_section_readback[2] = {0.0, 0.0};
    double intermediate_durations[1] = {0.0};
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};
    ruckig_position_extrema_t extrema[1];
    double first_time = 0.0;
    bool found = false;
    size_t section = 99;
    size_t allocations_before = 0;
    double zero_derivative_segment_duration = 0.0;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_create(&section_otg, 1, 0.05), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_get_max_number_of_waypoints(otg), 2);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&section_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&section_trajectory, 1), RUCKIG_WORKING);

    CHECK_TRUE(ruckig_input_max_position_data(input) != NULL);
    CHECK_TRUE(ruckig_input_min_position_data(input) != NULL);
    CHECK_TRUE(isinf(ruckig_input_max_position_const_data(input)[0]));
    CHECK_TRUE(isinf(ruckig_input_min_position_const_data(input)[0]));
    ruckig_input_max_position_data(input)[0] = 3.0;
    ruckig_input_min_position_data(input)[0] = -1.0;

    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_get_intermediate_position_count(input), 1);
    CHECK_EQ_INT(ruckig_input_get_intermediate_positions(input, waypoint_readback, 1), RUCKIG_WORKING);
    CHECK_NEAR(waypoint_readback[0], 1.0, 0.0);

    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 2), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_input_has_per_section_max_velocity(input));
    CHECK_TRUE(ruckig_input_has_per_section_minimum_duration(input));
    CHECK_EQ_INT(ruckig_input_get_per_section_max_velocity(input, per_section_readback, 2), RUCKIG_WORKING);
    CHECK_NEAR(per_section_readback[0], 1.2, 0.0);
    CHECK_NEAR(per_section_readback[1], 1.0, 0.0);

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 1.2;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 4.0;

    ruckig_input_current_position_data(section_input)[0] = 0.0;
    ruckig_input_target_position_data(section_input)[0] = 1.0;
    ruckig_input_max_velocity_data(section_input)[0] = per_section_max_velocity[0];
    CHECK_EQ_INT(ruckig_input_set_min_velocity(section_input, per_section_min_velocity, 1), RUCKIG_WORKING);
    ruckig_input_max_acceleration_data(section_input)[0] = per_section_max_acceleration[0];
    CHECK_EQ_INT(ruckig_input_set_min_acceleration(section_input, per_section_min_acceleration, 1), RUCKIG_WORKING);
    ruckig_input_max_jerk_data(section_input)[0] = per_section_max_jerk[0];
    CHECK_EQ_INT(ruckig_calculate(section_otg, section_input, section_trajectory), RUCKIG_WORKING);
    zero_derivative_segment_duration += ruckig_trajectory_get_duration(section_trajectory);

    ruckig_input_current_position_data(section_input)[0] = 1.0;
    ruckig_input_target_position_data(section_input)[0] = 2.0;
    ruckig_input_max_velocity_data(section_input)[0] = per_section_max_velocity[1];
    {
        double min_velocity[1] = {per_section_min_velocity[1]};
        double min_acceleration[1] = {per_section_min_acceleration[1]};
        CHECK_EQ_INT(ruckig_input_set_min_velocity(section_input, min_velocity, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_min_acceleration(section_input, min_acceleration, 1), RUCKIG_WORKING);
    }
    ruckig_input_max_acceleration_data(section_input)[0] = per_section_max_acceleration[1];
    ruckig_input_max_jerk_data(section_input)[0] = per_section_max_jerk[1];
    CHECK_EQ_INT(ruckig_calculate(section_otg, section_input, section_trajectory), RUCKIG_WORKING);
    zero_derivative_segment_duration += ruckig_trajectory_get_duration(section_trajectory);

    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_WORKING);
    ruckig_allocation_counters_reset();
    allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    ruckig_allocation_forbidden_set(false);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    CHECK_EQ_INT(ruckig_trajectory_get_section_count(trajectory), 2);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_duration_count(trajectory), 1);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, intermediate_durations, 1), RUCKIG_WORKING);
    CHECK_TRUE(intermediate_durations[0] > 0.0);
    CHECK_TRUE(intermediate_durations[0] < ruckig_trajectory_get_duration(trajectory));

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, intermediate_durations[0], position, velocity, acceleration, NULL, &section), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0, 1e-7);
    CHECK_EQ_INT(section, 1);
    CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) <= zero_derivative_segment_duration + 1e-9);
    CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, 1), RUCKIG_WORKING);
    CHECK_TRUE(extrema[0].min_position >= -1e-9);
    CHECK_TRUE(extrema[0].max_position <= 2.0 + 1e-9);
    CHECK_EQ_INT(ruckig_trajectory_get_first_time_at_position(trajectory, 0, 1.0, 0.0, &first_time, &found), RUCKIG_WORKING);
    CHECK_TRUE(found);
    CHECK_NEAR(first_time, intermediate_durations[0], 1e-7);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));

    ruckig_trajectory_destroy(section_trajectory);
    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(section_input);
    ruckig_input_destroy(input);
    ruckig_destroy(section_otg);
    ruckig_destroy(otg);
}

static void test_waypoint_validation_and_filter(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double waypoints[3] = {1.0, 2.0, 3.0};
    double filtered[3] = {0.0, 0.0, 0.0};
    double threshold[1] = {0.25};
    double short_waypoint[1] = {1.0};
    double valid_minimum_durations[4] = {0.0, 0.0, 0.0, 0.0};
    size_t written = 99;
    ruckig_control_interface_t per_control[1] = {RUCKIG_CONTROL_POSITION};

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 3), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 3, 1), RUCKIG_WORKING);
    ruckig_input_target_position_data(input)[0] = 4.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 2.0;

    CHECK_EQ_INT(ruckig_filter_intermediate_positions(otg, input, threshold, 1, filtered, 3, &written), RUCKIG_WORKING);
    CHECK_EQ_INT(written, 0);

    CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_DISCRETE), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_CONTINUOUS), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_minimum_duration(input, 1.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_clear_minimum_duration(input);

    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(input, per_control, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_clear_per_dof_control_interface(input);

    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_max_jerk_data(input)[0] = 2.0;

    {
        double invalid_max_velocity[4] = {1.0, 1.0, -1.0, 1.0};
        double invalid_min_velocity[4] = {-1.0, -1.0, 0.5, -1.0};
        double invalid_max_jerk[4] = {2.0, 2.0, INFINITY, 2.0};
        double invalid_max_position[4] = {2.0, 2.0, 2.0, 2.0};
        double invalid_min_position[4] = {-1.0, -1.0, 3.0, -1.0};
        CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, invalid_max_velocity, 4, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
        ruckig_input_clear_per_section_max_velocity(input);

        CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, invalid_min_velocity, 4, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
        ruckig_input_clear_per_section_min_velocity(input);

        CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, invalid_max_jerk, 4, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
        ruckig_input_clear_per_section_max_jerk(input);

        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, invalid_max_position, 4, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, invalid_min_position, 4, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
        ruckig_input_clear_per_section_max_position(input);
        ruckig_input_clear_per_section_min_position(input);
    }

    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, valid_minimum_durations, 4), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_input_has_per_section_minimum_duration(input));
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, short_waypoint, 1, 1), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_input_has_per_section_minimum_duration(input));
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 3, 1), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 0, false), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 0, true), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

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
        for (sample_index = 1; sample_index <= 5; ++sample_index) {
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
            CHECK_EQ_INT(sampled_section, section);
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
                if (per_section_max_jerk && isfinite(per_section_max_jerk[index])) {
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

static void check_alpha2_resume_trajectory(const ruckig_trajectory_t* trajectory) {
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

    CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) > 0.0);
    check_waypoint_samples(trajectory, waypoints, 2, dofs);
    check_waypoint_section_sampled_limits(
        trajectory,
        per_section_min_velocity,
        per_section_max_velocity,
        per_section_min_acceleration,
        per_section_max_acceleration,
        per_section_max_jerk,
        per_section_min_position,
        per_section_max_position,
        3,
        dofs);
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

static void test_waypoint_soft_interruption_update(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_trajectory_t* reference = NULL;
        ruckig_result_t result;
        double reference_duration = 0.0;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&reference, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        ruckig_input_clear_interrupt_calculation_duration(input);
        CHECK_EQ_INT(ruckig_calculate(otg, input, reference), RUCKIG_WORKING);
        reference_duration = ruckig_trajectory_get_duration(reference);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations > 1);
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), reference_duration, 1e-12);
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_reset(otg);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations > 1);
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), reference_duration, 1e-12);
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_trajectory_destroy(reference);
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);

        ruckig_output_pass_to_input(output, input);
        otg->waypoint_engine.best_duration = -1.0;
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);

        ruckig_output_pass_to_input(output, input);
        ruckig_input_clear_interrupt_calculation_duration(input);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.active);
        ruckig_output_pass_to_input(output, input);
        ruckig_input_current_position_data(input)[0] += 0.01;
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);

        ruckig_reset(otg);
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        const ruckig_trajectory_t* trajectory = NULL;
        double waypoint[1] = {1.0};
        double position[1] = {0.0};
        double duration = 0.0;
        size_t allocations_before = 0;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        ruckig_allocation_counters_reset();
        allocations_before = ruckig_allocation_count();
        ruckig_allocation_forbidden_set(true);
        result = ruckig_update(otg, input, output);
        ruckig_allocation_forbidden_set(false);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
        CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_TRUE(!otg->waypoint_engine.complete);

        trajectory = ruckig_output_get_trajectory(output);
        duration = ruckig_trajectory_get_duration(trajectory);
        CHECK_TRUE(duration > 0.0);
        check_waypoint_samples(trajectory, waypoint, 1, 1);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, duration, position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], 2.0, 1e-7);

        ruckig_output_pass_to_input(output, input);
        allocations_before = ruckig_allocation_count();
        ruckig_allocation_forbidden_set(true);
        result = ruckig_update(otg, input, output);
        ruckig_allocation_forbidden_set(false);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
        CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_NEAR(ruckig_output_get_time(output), 0.05, 1e-12);
        trajectory = ruckig_output_get_trajectory(output);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) > 0.0);
        check_waypoint_samples(trajectory, waypoint, 1, 1);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        double waypoint[1] = {1.0};
        double per_section_max_position[2] = {0.5, 2.5};
        double per_section_min_position[2] = {-0.5, 0.5};
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        ruckig_input_target_position_data(input)[0] = 2.0;
        ruckig_input_max_velocity_data(input)[0] = 1.2;
        ruckig_input_max_acceleration_data(input)[0] = 2.0;
        ruckig_input_max_jerk_data(input)[0] = 4.0;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_ERROR_EXECUTION_TIME_CALCULATION);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), 0.0, 0.0);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;

        CHECK_EQ_INT(ruckig_create(&otg, 1, 0.05), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
        ruckig_input_target_position_data(input)[0] = 1.0;
        ruckig_input_max_velocity_data(input)[0] = 1.0;
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoint[1] = {1.0};

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations > 1);
        check_waypoint_samples(trajectory, waypoint, 1, 1);

        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

typedef enum waypoint_alpha2_invalidation_kind {
    WAYPOINT_ALPHA2_INVALIDATE_TARGET = 0,
    WAYPOINT_ALPHA2_INVALIDATE_WAYPOINTS,
    WAYPOINT_ALPHA2_INVALIDATE_WAYPOINT_COUNT,
    WAYPOINT_ALPHA2_INVALIDATE_LIMITS,
    WAYPOINT_ALPHA2_INVALIDATE_PER_SECTION,
    WAYPOINT_ALPHA2_INVALIDATE_ENABLED_DOF,
    WAYPOINT_ALPHA2_INVALIDATE_SYNCHRONIZATION,
    WAYPOINT_ALPHA2_INVALIDATE_CONTROL_INTERFACE,
    WAYPOINT_ALPHA2_INVALIDATE_DURATION_DISCRETIZATION,
    WAYPOINT_ALPHA2_INVALIDATE_CLEAR_INTERRUPT
} waypoint_alpha2_invalidation_kind_t;

static void configure_alpha2_invalidation_input(ruckig_input_t* input) {
    double waypoints[4] = {0.40, 0.0, 0.82, 0.0};
    size_t i;
    for (i = 0; i < 2; ++i) {
        ruckig_input_max_velocity_data(input)[i] = 1.2;
        ruckig_input_max_acceleration_data(input)[i] = 2.0;
        ruckig_input_max_jerk_data(input)[i] = 4.0;
    }
    ruckig_input_target_position_data(input)[0] = 1.20;
    ruckig_input_target_position_data(input)[1] = 0.0;
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, 2), RUCKIG_WORKING);
}

static void apply_alpha2_invalidation(ruckig_input_t* input, waypoint_alpha2_invalidation_kind_t kind) {
    switch (kind) {
    case WAYPOINT_ALPHA2_INVALIDATE_TARGET:
        ruckig_input_target_position_data(input)[0] += 0.10;
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_WAYPOINTS: {
        double waypoints[4] = {0.45, 0.0, 0.90, 0.0};
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, 2), RUCKIG_WORKING);
        break;
    }
    case WAYPOINT_ALPHA2_INVALIDATE_WAYPOINT_COUNT: {
        double waypoint[2] = {0.55, 0.0};
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 2), RUCKIG_WORKING);
        break;
    }
    case WAYPOINT_ALPHA2_INVALIDATE_LIMITS:
        ruckig_input_max_velocity_data(input)[0] = 1.05;
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_PER_SECTION: {
        double per_section_minimum_duration[3] = {0.08, 0.12, 0.08};
        CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 3), RUCKIG_WORKING);
        break;
    }
    case WAYPOINT_ALPHA2_INVALIDATE_ENABLED_DOF:
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 1, false), RUCKIG_WORKING);
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_SYNCHRONIZATION:
        CHECK_EQ_INT(ruckig_input_set_synchronization(input, RUCKIG_SYNCHRONIZATION_NONE), RUCKIG_WORKING);
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_CONTROL_INTERFACE:
        CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY), RUCKIG_WORKING);
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_DURATION_DISCRETIZATION:
        CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_DISCRETE), RUCKIG_WORKING);
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_CLEAR_INTERRUPT:
        ruckig_input_clear_interrupt_calculation_duration(input);
        break;
    }
}

static void check_alpha2_invalidation_case(
    waypoint_alpha2_invalidation_kind_t kind,
    ruckig_result_t expected_result
) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 2, 0.02, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 2, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 2, 2), RUCKIG_WORKING);
    configure_alpha2_invalidation_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(otg->waypoint_engine.active);
    ruckig_output_pass_to_input(output, input);

    apply_alpha2_invalidation(input, kind);
    result = ruckig_update(otg, input, output);
    CHECK_EQ_INT(result, expected_result);
    if (kind == WAYPOINT_ALPHA2_INVALIDATE_CLEAR_INTERRUPT) {
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
    } else if (expected_result == RUCKIG_WORKING) {
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);
    } else {
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
    }

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_waypoint_soft_interruption_alpha2_hardening(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double reference_duration = 0.0;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 3, 2), RUCKIG_WORKING);
        configure_alpha2_resume_input(input);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations > 3);
        reference_duration = ruckig_trajectory_get_duration(trajectory);
        check_alpha2_resume_trajectory(trajectory);

        ruckig_input_clear_interrupt_calculation_duration(input);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), reference_duration, 1e-9);
        check_alpha2_resume_trajectory(ruckig_output_get_trajectory(output));

        ruckig_trajectory_destroy(trajectory);
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_t* fresh_otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_input_t* fresh_input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_trajectory_t* fresh_trajectory = NULL;
        double incumbent_remaining_duration;
        double fresh_duration;
        double published_duration;
        size_t allocations_before;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_create_with_waypoints(&fresh_otg, 3, 0.02, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&fresh_input, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&fresh_trajectory, 3, 2), RUCKIG_WORKING);
        configure_alpha2_resume_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);
        check_alpha2_resume_trajectory(ruckig_output_get_trajectory(output));

        incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_copy_state(input, fresh_input), RUCKIG_WORKING);
        ruckig_input_clear_interrupt_calculation_duration(fresh_input);
        CHECK_EQ_INT(ruckig_calculate(fresh_otg, fresh_input, fresh_trajectory), RUCKIG_WORKING);
        fresh_duration = ruckig_trajectory_get_duration(fresh_trajectory);
        CHECK_TRUE(fresh_duration > 0.0);
        CHECK_TRUE(fresh_duration <= incumbent_remaining_duration + 1e-9);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        ruckig_allocation_counters_reset();
        allocations_before = ruckig_allocation_count();
        ruckig_allocation_forbidden_set(true);
        result = ruckig_update(otg, input, output);
        ruckig_allocation_forbidden_set(false);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
        CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_NEAR(ruckig_output_get_time(output), 0.02, 1e-12);
        published_duration = ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output));
        CHECK_TRUE(published_duration < incumbent_remaining_duration - 1e-12);
        check_alpha2_resume_trajectory(ruckig_output_get_trajectory(output));

        ruckig_trajectory_destroy(fresh_trajectory);
        ruckig_output_destroy(output);
        ruckig_input_destroy(fresh_input);
        ruckig_input_destroy(input);
        ruckig_destroy(fresh_otg);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        size_t cycle;
        bool saw_background_publish = false;
        bool saw_interrupted_without_publish = false;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
        configure_alpha2_resume_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);

        for (cycle = 0; cycle < 24; ++cycle) {
            const double previous_time = ruckig_output_get_time(output);
            const double incumbent_remaining_duration =
                ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - previous_time;
            ruckig_output_pass_to_input(output, input);
            CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
            CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations <= 1);
            if (ruckig_output_new_calculation(output)) {
                CHECK_NEAR(ruckig_output_get_time(output), 0.02, 1e-12);
                CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                    < incumbent_remaining_duration - 1e-12);
                saw_background_publish = true;
            } else {
                CHECK_TRUE(ruckig_output_get_time(output) > previous_time);
                if (ruckig_output_was_calculation_interrupted(output)) {
                    saw_interrupted_without_publish = true;
                }
            }
            check_alpha2_resume_trajectory(ruckig_output_get_trajectory(output));
        }
        CHECK_TRUE(saw_background_publish);
        CHECK_TRUE(saw_interrupted_without_publish);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        double* target_position;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_EQ_INT(ruckig_calculate(otg, input, output->trajectory), RUCKIG_WORKING);
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations > 1);

        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, NULL, 0, 1), RUCKIG_WORKING);
        target_position = ruckig_input_target_position_data(input);
        target_position[0] = 2.5;
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_TARGET, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_WAYPOINTS, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_WAYPOINT_COUNT, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_LIMITS, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_PER_SECTION, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_ENABLED_DOF, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_SYNCHRONIZATION, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_CONTROL_INTERFACE, RUCKIG_ERROR_INVALID_INPUT);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_DURATION_DISCRETIZATION, RUCKIG_ERROR_INVALID_INPUT);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_CLEAR_INTERRUPT, RUCKIG_WORKING);
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

void run_waypoint_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    test_waypoint_validation_and_filter();
    test_waypoint_soft_interruption_update();
    test_waypoint_soft_interruption_alpha2_hardening();
    run_waypoint_all_regression_tests();
}

void run_waypoint_per_section_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    run_waypoint_per_section_regression_tests();
}

void run_waypoint_quality_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    run_waypoint_quality_regression_tests();
}
