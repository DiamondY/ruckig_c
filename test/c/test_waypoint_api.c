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

static void check_waypoint_samples(
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

static void check_waypoint_section_sampled_limits(
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

#define WAYPOINT_RESUME_QUALITY_CASES 128u

typedef struct waypoint_resume_quality_baseline {
    ruckig_result_t result;
    double duration;
} waypoint_resume_quality_baseline_t;

static const waypoint_resume_quality_baseline_t waypoint_resume_quality_baseline[WAYPOINT_RESUME_QUALITY_CASES] = {
    {RUCKIG_WORKING, 1.8761222240029074},
    {RUCKIG_WORKING, 2.0103531913159798},
    {RUCKIG_WORKING, 2.1367785811810092},
    {RUCKIG_WORKING, 2.3231046365914789},
    {RUCKIG_WORKING, 1.9563060529630401},
    {RUCKIG_WORKING, 2.1639081016881558},
    {RUCKIG_WORKING, 2.334308960646033},
    {RUCKIG_WORKING, 2.463331019114928},
    {RUCKIG_WORKING, 2.0843835602493597},
    {RUCKIG_WORKING, 2.9613331216506076},
    {RUCKIG_WORKING, 3.0447526376853462},
    {RUCKIG_WORKING, 2.9220220742529053},
    {RUCKIG_WORKING, 1.8927697784843993},
    {RUCKIG_WORKING, 2.0322608374341988},
    {RUCKIG_WORKING, 2.1567261904761903},
    {RUCKIG_WORKING, 2.2663324979114456},
    {RUCKIG_WORKING, 1.9684155547745192},
    {RUCKIG_WORKING, 2.1692990031800696},
    {RUCKIG_WORKING, 2.5305866703295874},
    {RUCKIG_WORKING, 2.4550580647027891},
    {RUCKIG_WORKING, 2.0569052768441773},
    {RUCKIG_WORKING, 2.7292669721409863},
    {RUCKIG_WORKING, 3.0123994118392505},
    {RUCKIG_WORKING, 2.8644266241023915},
    {RUCKIG_WORKING, 1.9245663979634555},
    {RUCKIG_WORKING, 2.0413240040177811},
    {RUCKIG_WORKING, 2.1612818916534553},
    {RUCKIG_WORKING, 2.4002506265664163},
    {RUCKIG_WORKING, 1.977557379612221},
    {RUCKIG_WORKING, 2.2563187996268708},
    {RUCKIG_WORKING, 2.4584710349512853},
    {RUCKIG_WORKING, 2.4222272872301414},
    {RUCKIG_WORKING, 2.1047787855405429},
    {RUCKIG_WORKING, 2.4657343587793634},
    {RUCKIG_WORKING, 3.1531469118030202},
    {RUCKIG_WORKING, 2.6061641302819289},
    {RUCKIG_WORKING, 1.9239972254420541},
    {RUCKIG_WORKING, 2.0558384224789195},
    {RUCKIG_WORKING, 2.2248443577610386},
    {RUCKIG_WORKING, 2.3223942208462334},
    {RUCKIG_WORKING, 2.0283780734336379},
    {RUCKIG_WORKING, 2.2574411257868867},
    {RUCKIG_WORKING, 2.4644611096425031},
    {RUCKIG_WORKING, 2.4457278825371329},
    {RUCKIG_WORKING, 2.0848511936006613},
    {RUCKIG_WORKING, 2.6054809401039547},
    {RUCKIG_WORKING, 3.0800045689903248},
    {RUCKIG_WORKING, 2.8832218947413302},
    {RUCKIG_WORKING, 1.9160505589883272},
    {RUCKIG_WORKING, 2.0760672687280288},
    {RUCKIG_WORKING, 2.1888431138892592},
    {RUCKIG_WORKING, 2.2999908098908737},
    {RUCKIG_WORKING, 2.0038814140165444},
    {RUCKIG_WORKING, 2.3860493642534069},
    {RUCKIG_WORKING, 2.557441802528555},
    {RUCKIG_WORKING, 2.23899929243351},
    {RUCKIG_WORKING, 2.0428579478127338},
    {RUCKIG_WORKING, 2.3215913079831605},
    {RUCKIG_WORKING, 3.4106228864090751},
    {RUCKIG_WORKING, 2.8149811574204939},
    {RUCKIG_WORKING, 1.9318541856703626},
    {RUCKIG_WORKING, 2.0522195091848081},
    {RUCKIG_WORKING, 2.0631838839490344},
    {RUCKIG_WORKING, 2.3900689223057645},
    {RUCKIG_WORKING, 2.0494105941215643},
    {RUCKIG_WORKING, 2.5094798441135246},
    {RUCKIG_WORKING, 2.2752288475778109},
    {RUCKIG_WORKING, 2.5289410155537513},
    {RUCKIG_WORKING, 2.0357220222917145},
    {RUCKIG_WORKING, 2.0774397928966808},
    {RUCKIG_WORKING, 3.0343783763481187},
    {RUCKIG_WORKING, 2.7125519658727493},
    {RUCKIG_WORKING, 1.9442812786899282},
    {RUCKIG_WORKING, 2.069495314403671},
    {RUCKIG_WORKING, 2.2236904761904763},
    {RUCKIG_WORKING, 2.3257881283373774},
    {RUCKIG_WORKING, 2.0353605455021526},
    {RUCKIG_WORKING, 2.4109810151649858},
    {RUCKIG_WORKING, 2.4176495882389277},
    {RUCKIG_WORKING, 2.4860267003990359},
    {RUCKIG_WORKING, 2.0510236404242619},
    {RUCKIG_WORKING, 2.6080382297388756},
    {RUCKIG_WORKING, 3.091791134072011},
    {RUCKIG_WORKING, 2.903038931573815},
    {RUCKIG_WORKING, 1.9702790260533687},
    {RUCKIG_WORKING, 2.083353074399418},
    {RUCKIG_WORKING, 2.1970749992119081},
    {RUCKIG_WORKING, 2.4716791979949875},
    {RUCKIG_WORKING, 1.978703111129096},
    {RUCKIG_WORKING, 2.0772403453156212},
    {RUCKIG_WORKING, 2.3591172672337084},
    {RUCKIG_WORKING, 2.3636957636010121},
    {RUCKIG_WORKING, 2.0532460636728844},
    {RUCKIG_WORKING, 2.5913216388654354},
    {RUCKIG_WORKING, 3.3428153457599308},
    {RUCKIG_WORKING, 2.54395314189289},
    {RUCKIG_WORKING, 1.9754432000429079},
    {RUCKIG_WORKING, 2.0936269995027805},
    {RUCKIG_WORKING, 2.2964285714285708},
    {RUCKIG_WORKING, 2.2466345402602599},
    {RUCKIG_WORKING, 1.9142690602950223},
    {RUCKIG_WORKING, 2.177528236611459},
    {RUCKIG_WORKING, 2.3408605072052615},
    {RUCKIG_WORKING, 2.5460348885692179},
    {RUCKIG_WORKING, 2.0512234950866777},
    {RUCKIG_WORKING, 2.7281909321239359},
    {RUCKIG_WORKING, 3.1320961226633219},
    {RUCKIG_WORKING, 2.8955176361553034},
    {RUCKIG_WORKING, 1.9944569522369353},
    {RUCKIG_WORKING, 2.1238095238095238},
    {RUCKIG_WORKING, 2.1104950014176227},
    {RUCKIG_WORKING, 2.2762819997371411},
    {RUCKIG_WORKING, 1.9421733108693535},
    {RUCKIG_WORKING, 2.4233387413110896},
    {RUCKIG_WORKING, 2.2582695351593518},
    {RUCKIG_WORKING, 2.4168334927430806},
    {RUCKIG_WORKING, 2.0789756193223998},
    {RUCKIG_WORKING, 2.5650590147225167},
    {RUCKIG_WORKING, 3.335222010769173},
    {RUCKIG_WORKING, 2.8060429882502538},
    {RUCKIG_WORKING, 1.9847952137456903},
    {RUCKIG_WORKING, 1.9957062475352765},
    {RUCKIG_WORKING, 2.1236601897753999},
    {RUCKIG_WORKING, 2.3097117794486217},
    {RUCKIG_WORKING, 1.998232074574996},
    {RUCKIG_WORKING, 2.2494421209405626},
    {RUCKIG_WORKING, 2.3453201943745858},
    {RUCKIG_WORKING, 2.5133170309898056}
};

static void configure_waypoint_resume_quality_case(
    ruckig_input_t* input,
    size_t case_id,
    double* waypoints_out
) {
    const size_t dofs = input->dofs;
    const size_t waypoint_count = input->max_number_of_waypoints;
    const size_t section_count = waypoint_count + 1;
    double waypoints[12] = {0.0};
    double per_section_min_velocity[16] = {0.0};
    double per_section_max_velocity[16] = {0.0};
    double per_section_min_acceleration[16] = {0.0};
    double per_section_max_acceleration[16] = {0.0};
    double per_section_max_jerk[16] = {0.0};
    double per_section_min_position[16] = {0.0};
    double per_section_max_position[16] = {0.0};
    double per_section_minimum_duration[4] = {0.0};
    size_t waypoint;
    size_t section;
    size_t dof;

    CHECK_EQ_INT(ruckig_input_set_synchronization(
        input,
        (ruckig_synchronization_t)(case_id % 4u)), RUCKIG_WORKING);

    for (dof = 0; dof < dofs; ++dof) {
        const bool disabled = dofs > 1 && ((case_id + dof * 7u) % 19u == 0u);
        const double sign = ((case_id + dof) % 2u) == 0u ? 1.0 : -1.0;
        const double distance = disabled ? 0.0 : sign * (0.85 + 0.18 * (double)dof + 0.015 * (double)(case_id % 11u));
        ruckig_input_current_position_data(input)[dof] = 0.0;
        ruckig_input_current_velocity_data(input)[dof] = 0.0;
        ruckig_input_current_acceleration_data(input)[dof] = 0.0;
        ruckig_input_target_position_data(input)[dof] = distance;
        ruckig_input_target_velocity_data(input)[dof] = 0.0;
        ruckig_input_target_acceleration_data(input)[dof] = 0.0;
        ruckig_input_max_velocity_data(input)[dof] = 1.05 + 0.07 * (double)((case_id + dof) % 5u);
        ruckig_input_max_acceleration_data(input)[dof] = 1.80 + 0.10 * (double)((case_id + 2u * dof) % 4u);
        ruckig_input_max_jerk_data(input)[dof] = 4.20 + 0.20 * (double)((case_id + 3u * dof) % 4u);
        ruckig_input_max_position_data(input)[dof] = distance >= 0.0 ? distance + 0.75 : 0.75;
        ruckig_input_min_position_data(input)[dof] = distance >= 0.0 ? -0.75 : distance - 0.75;
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, dof, !disabled), RUCKIG_WORKING);
    }

    for (waypoint = 0; waypoint < waypoint_count; ++waypoint) {
        const double fraction = (double)(waypoint + 1u) / (double)(waypoint_count + 1u);
        for (dof = 0; dof < dofs; ++dof) {
            const bool enabled = ruckig_input_enabled_const_data(input)[dof];
            const double target = ruckig_input_target_position_const_data(input)[dof];
            const double curvature = enabled
                ? 0.025 * (double)(((case_id + waypoint * 3u + dof) % 3u) + 1u)
                    * (target >= 0.0 ? 1.0 : -1.0)
                : 0.0;
            double value = target * fraction + curvature * sin((double)(waypoint + 1u));
            if (fabs(value) > fabs(target) && target != 0.0) {
                value = target * fraction;
            }
            waypoints[waypoint * dofs + dof] = enabled ? value : 0.0;
        }
    }
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, waypoint_count, dofs), RUCKIG_WORKING);
    if (waypoints_out) {
        memcpy(waypoints_out, waypoints, sizeof(double) * waypoint_count * dofs);
    }

    for (section = 0; section < section_count; ++section) {
        const double start_fraction = (double)section / (double)section_count;
        const double end_fraction = (double)(section + 1u) / (double)section_count;
        per_section_minimum_duration[section] = 0.04 + 0.01 * (double)((case_id + section) % 3u);
        for (dof = 0; dof < dofs; ++dof) {
            const size_t index = section * dofs + dof;
            const double target = ruckig_input_target_position_const_data(input)[dof];
            const double lo = target >= 0.0 ? target * start_fraction - 0.55 : target * end_fraction - 0.55;
            const double hi = target >= 0.0 ? target * end_fraction + 0.55 : target * start_fraction + 0.55;
            const double max_velocity = ruckig_input_max_velocity_const_data(input)[dof];
            const double max_acceleration = ruckig_input_max_acceleration_const_data(input)[dof];
            per_section_min_velocity[index] = -max_velocity;
            per_section_max_velocity[index] = max_velocity;
            per_section_min_acceleration[index] = -max_acceleration;
            per_section_max_acceleration[index] = max_acceleration;
            per_section_max_jerk[index] = ruckig_input_max_jerk_const_data(input)[dof];
            per_section_min_position[index] = lo;
            per_section_max_position[index] = hi;
        }
    }

    if ((case_id % 2u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, section_count, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 3u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, section_count, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 5u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 7u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, section_count, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 11u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, section_count), RUCKIG_WORKING);
    }
}

static ruckig_result_t waypoint_resume_quality_calculate_case(
    size_t case_id,
    ruckig_trajectory_t** trajectory_out
) {
    const size_t dofs = 1u + (case_id % 4u);
    const size_t waypoint_count = 1u + ((case_id / 4u) % 3u);
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, waypoint_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, waypoint_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, waypoint_count), RUCKIG_WORKING);
    configure_waypoint_resume_quality_case(input, case_id, NULL);

    result = ruckig_calculate(otg, input, trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    if (trajectory_out) {
        *trajectory_out = trajectory;
    } else {
        ruckig_trajectory_destroy(trajectory);
    }
    return result;
}

void run_waypoint_resume_quality_baseline_dump(void) {
    size_t case_id;
    printf("static const waypoint_resume_quality_baseline_t waypoint_resume_quality_baseline[WAYPOINT_RESUME_QUALITY_CASES] = {\n");
    for (case_id = 0; case_id < WAYPOINT_RESUME_QUALITY_CASES; ++case_id) {
        ruckig_trajectory_t* trajectory = NULL;
        const ruckig_result_t result = waypoint_resume_quality_calculate_case(case_id, &trajectory);
        const double duration = result == RUCKIG_WORKING ? ruckig_trajectory_get_duration(trajectory) : -1.0;
        printf("    {%s, %.17g}%s\n",
            result == RUCKIG_WORKING ? "RUCKIG_WORKING" : "RUCKIG_ERROR",
            duration,
            case_id + 1u == WAYPOINT_RESUME_QUALITY_CASES ? "" : ",");
        ruckig_trajectory_destroy(trajectory);
    }
    printf("};\n");
}

void run_waypoint_resume_quality_audit_tests(void) {
    double max_regression = 0.0;
    double sum_ratio = 0.0;
    size_t successful_cases = 0;
    size_t publish_count = 0;
    size_t interrupted_without_publish_count = 0;
    size_t completion_count = 0;
    size_t fresh_reference_count = 0;
    size_t case_id;

    for (case_id = 0; case_id < WAYPOINT_RESUME_QUALITY_CASES; ++case_id) {
        ruckig_trajectory_t* trajectory = NULL;
        const ruckig_result_t result = waypoint_resume_quality_calculate_case(case_id, &trajectory);
        const waypoint_resume_quality_baseline_t baseline = waypoint_resume_quality_baseline[case_id];
        CHECK_EQ_INT(result, baseline.result);
        if (result == RUCKIG_WORKING) {
            const double duration = ruckig_trajectory_get_duration(trajectory);
            const double regression = duration - baseline.duration;
            CHECK_TRUE(duration > 0.0);
            CHECK_TRUE(duration <= baseline.duration + 1.0e-9);
            if (regression > max_regression) {
                max_regression = regression;
            }
            sum_ratio += duration / baseline.duration;
            ++successful_cases;
        }
        ruckig_trajectory_destroy(trajectory);
    }

    for (case_id = 0; case_id < WAYPOINT_RESUME_QUALITY_CASES; case_id += 4u) {
        const size_t dofs = 1u + (case_id % 4u);
        const size_t waypoint_count = 1u + ((case_id / 4u) % 3u);
        ruckig_t* otg = NULL;
        ruckig_t* fresh_otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_input_t* fresh_input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_trajectory_t* fresh_trajectory = NULL;
        size_t cycle;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_create_with_waypoints(&fresh_otg, dofs, 0.01, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&fresh_input, dofs, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, dofs, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&fresh_trajectory, dofs, waypoint_count), RUCKIG_WORKING);
        configure_waypoint_resume_quality_case(input, case_id, NULL);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);

        for (cycle = 0; cycle < 40; ++cycle) {
            const double previous_time = ruckig_output_get_time(output);
            const double incumbent_remaining_duration =
                ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - previous_time;
            const bool was_active = otg->waypoint_engine.active;
            ruckig_output_pass_to_input(output, input);
            if (cycle == 5u) {
                CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1.0), RUCKIG_WORKING);
            } else if (cycle == 6u) {
                CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
            } else if (cycle == 20u) {
                CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
            }
            if (was_active) {
                CHECK_EQ_INT(ruckig_input_copy_state(input, fresh_input), RUCKIG_WORKING);
                ruckig_input_clear_interrupt_calculation_duration(fresh_input);
                if (ruckig_calculate(fresh_otg, fresh_input, fresh_trajectory) == RUCKIG_WORKING) {
                    CHECK_TRUE(ruckig_trajectory_get_duration(fresh_trajectory) > 0.0);
                    ++fresh_reference_count;
                }
            }
            CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) > 0.0);
            if (ruckig_output_new_calculation(output)) {
                CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
                CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                    < incumbent_remaining_duration - 1.0e-12);
                ++publish_count;
            } else {
                CHECK_TRUE(ruckig_output_get_time(output) > previous_time);
                if (ruckig_output_was_calculation_interrupted(output)) {
                    ++interrupted_without_publish_count;
                }
            }
            if (was_active && !otg->waypoint_engine.active) {
                ++completion_count;
                break;
            }
        }

        ruckig_trajectory_destroy(fresh_trajectory);
        ruckig_output_destroy(output);
        ruckig_input_destroy(fresh_input);
        ruckig_input_destroy(input);
        ruckig_destroy(fresh_otg);
        ruckig_destroy(otg);
    }

    CHECK_TRUE(successful_cases > 0);
    CHECK_TRUE(publish_count > 0);
    CHECK_TRUE(interrupted_without_publish_count > 0);
    CHECK_TRUE(completion_count > 0);
    CHECK_TRUE(fresh_reference_count > 0);
    printf("waypoint resume quality audit: cases %zu successful %zu avg_ratio %.12g max_regression %.12g publish %zu interrupted_without_publish %zu completion %zu fresh_reference %zu\n",
        (size_t)WAYPOINT_RESUME_QUALITY_CASES,
        successful_cases,
        sum_ratio / (double)successful_cases,
        max_regression,
        publish_count,
        interrupted_without_publish_count,
        completion_count,
        fresh_reference_count);
}

static void test_waypoint_resume_stress_budget_matrix(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        double incumbent_remaining_duration = 0.0;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
        configure_alpha1_resume_stress_input(input);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1.0), RUCKIG_WORKING);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations >= 1);
        if (ruckig_output_new_calculation(output)) {
            CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                < incumbent_remaining_duration - 1e-12);
        }
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        if (ruckig_output_new_calculation(output)) {
            CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                < incumbent_remaining_duration - 1e-12);
        }
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
        if (ruckig_output_new_calculation(output)) {
            CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                < incumbent_remaining_duration - 1e-12);
        }
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        const double previous_time = 0.0;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
        configure_alpha1_resume_stress_input(input);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.active);
        ruckig_output_pass_to_input(output, input);
        ruckig_input_clear_interrupt_calculation_duration(input);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_TRUE(ruckig_output_get_time(output) > previous_time);
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

static void test_waypoint_resume_stress_long_online_loop(void) {
    ruckig_t* otg = NULL;
    ruckig_t* fresh_otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_input_t* fresh_input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* fresh_trajectory = NULL;
    bool saw_publish = false;
    bool saw_fresh_quality_reference = false;
    bool saw_budget_interruption = false;
    bool saw_completion = false;
    size_t cycle;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_create_with_waypoints(&fresh_otg, 4, 0.01, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&fresh_input, 4, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&fresh_trajectory, 4, 3), RUCKIG_WORKING);
    configure_alpha1_resume_stress_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(otg->waypoint_engine.active);
    check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

    for (cycle = 0; cycle < 32; ++cycle) {
        const double previous_time = ruckig_output_get_time(output);
        const size_t previous_section = ruckig_output_get_new_section(output);
        const double incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - previous_time;
        const bool was_active_before_update = otg->waypoint_engine.active;
        double fresh_duration = 0.0;
        ruckig_result_t result;

        ruckig_output_pass_to_input(output, input);
        if (cycle == 2) {
            CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1.0), RUCKIG_WORKING);
        } else if (cycle == 3) {
            CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        } else if (cycle == 10) {
            CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        }

        if (was_active_before_update) {
            CHECK_EQ_INT(ruckig_input_copy_state(input, fresh_input), RUCKIG_WORKING);
            ruckig_input_clear_interrupt_calculation_duration(fresh_input);
            CHECK_EQ_INT(ruckig_calculate(fresh_otg, fresh_input, fresh_trajectory), RUCKIG_WORKING);
            fresh_duration = ruckig_trajectory_get_duration(fresh_trajectory);
            CHECK_TRUE(fresh_duration > 0.0);
            saw_fresh_quality_reference = true;
        }

        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_get_new_section(output) < 4);
        if (ruckig_output_new_calculation(output)) {
            const double published_duration = ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output));
            CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
            CHECK_TRUE(published_duration < incumbent_remaining_duration - 1e-12);
            CHECK_TRUE(published_duration > 0.0);
            saw_publish = true;
        } else {
            CHECK_TRUE(ruckig_output_get_time(output) > previous_time);
            CHECK_TRUE(ruckig_output_get_new_section(output) >= previous_section);
            if (ruckig_output_was_calculation_interrupted(output)) {
                saw_budget_interruption = true;
            }
        }
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        if (cycle >= 10 && was_active_before_update && !otg->waypoint_engine.active) {
            CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
            saw_completion = true;
            break;
        }
    }

    CHECK_TRUE(saw_fresh_quality_reference);
    CHECK_TRUE(saw_budget_interruption);
    CHECK_TRUE(saw_publish);
    CHECK_TRUE(saw_completion);

    ruckig_trajectory_destroy(fresh_trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(fresh_input);
    ruckig_input_destroy(input);
    ruckig_destroy(fresh_otg);
    ruckig_destroy(otg);
}

static void test_waypoint_resume_stress_allocation_paths(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
        configure_alpha1_resume_stress_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        ruckig_allocation_counters_reset();
        CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);

        ruckig_output_pass_to_input(output, input);
        otg->waypoint_engine.best_duration = -1.0;
        CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        double incumbent_remaining_duration = 0.0;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
        configure_alpha1_resume_stress_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.active);

        incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        ruckig_allocation_counters_reset();
        CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
        if (ruckig_output_new_calculation(output)) {
            CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                < incumbent_remaining_duration - 1e-12);
        }
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

static void test_waypoint_fixed_regression_corpus(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoint[2] = {1.0, -0.5};
        double intermediate_duration[1] = {0.0};
        double position[2] = {0.0, 0.0};

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 2, 0.01, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 2, 1), RUCKIG_WORKING);
        ruckig_input_target_position_data(input)[0] = 2.0;
        ruckig_input_target_position_data(input)[1] = -1.0;
        ruckig_input_max_velocity_data(input)[0] = 1.0;
        ruckig_input_max_velocity_data(input)[1] = 1.0;
        ruckig_input_max_acceleration_data(input)[0] = 2.0;
        ruckig_input_max_acceleration_data(input)[1] = 2.0;
        ruckig_input_max_jerk_data(input)[0] = 4.0;
        ruckig_input_max_jerk_data(input)[1] = 4.0;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 3.0, 1e-9);
        CHECK_EQ_INT(ruckig_trajectory_get_section_count(trajectory), 2);
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, intermediate_duration, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, intermediate_duration[0], position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], waypoint[0], 1e-7);
        CHECK_NEAR(position[1], waypoint[1], 1e-7);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoint[1] = {1.0};
        double per_section_max_velocity[2] = {0.8, 1.4};
        double per_section_min_velocity[2] = {-0.8, -1.4};
        double per_section_max_acceleration[2] = {1.2, 2.0};
        double per_section_min_acceleration[2] = {-1.2, -2.0};
        double per_section_max_jerk[2] = {3.0, 5.0};
        double per_section_max_position[2] = {1.1, 2.1};
        double per_section_min_position[2] = {-0.1, 0.9};
        ruckig_position_extrema_t extrema[1];

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.01, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 1), RUCKIG_WORKING);
        ruckig_input_target_position_data(input)[0] = 2.0;
        ruckig_input_max_velocity_data(input)[0] = 1.5;
        ruckig_input_max_acceleration_data(input)[0] = 2.0;
        ruckig_input_max_jerk_data(input)[0] = 5.0;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 3.210714722108343, 1e-9);
        CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, 1), RUCKIG_WORKING);
        CHECK_TRUE(extrema[0].min_position >= -1e-9);
        CHECK_TRUE(extrema[0].max_position <= 2.0 + 1e-9);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        const size_t dofs = 3;
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoints[6] = {0.5, -0.2, 0.25, 1.0, -0.4, 0.50};
        double durations[2] = {0.0, 0.0};
        double position[3] = {0.0, 0.0, 0.0};
        size_t i;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.02, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 2), RUCKIG_WORKING);
        for (i = 0; i < dofs; ++i) {
            ruckig_input_max_velocity_data(input)[i] = 1.5;
            ruckig_input_max_acceleration_data(input)[i] = 2.0;
            ruckig_input_max_jerk_data(input)[i] = 4.0;
        }
        ruckig_input_target_position_data(input)[0] = 1.5;
        ruckig_input_target_position_data(input)[1] = -0.6;
        ruckig_input_target_position_data(input)[2] = 0.75;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.617255105467443, 1e-9);
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 2), RUCKIG_WORKING);
        CHECK_TRUE(durations[0] > 0.0 && durations[0] < durations[1]);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, durations[1], position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], waypoints[3], 1e-7);
        CHECK_NEAR(position[1], waypoints[4], 1e-7);
        CHECK_NEAR(position[2], waypoints[5], 1e-7);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoint[2] = {1.0, 5.0};
        double intermediate_duration[1] = {0.0};
        double position[2] = {0.0, 0.0};

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 2, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 2, 1), RUCKIG_WORKING);
        ruckig_input_current_position_data(input)[1] = 5.0;
        ruckig_input_target_position_data(input)[0] = 2.0;
        ruckig_input_target_position_data(input)[1] = 5.0;
        ruckig_input_max_velocity_data(input)[0] = 1.2;
        ruckig_input_max_velocity_data(input)[1] = 1.0;
        ruckig_input_max_acceleration_data(input)[0] = 2.0;
        ruckig_input_max_acceleration_data(input)[1] = 1.0;
        ruckig_input_max_jerk_data(input)[0] = 4.0;
        ruckig_input_max_jerk_data(input)[1] = 2.0;
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 1, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, intermediate_duration, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, intermediate_duration[0], position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], 1.0, 1e-7);
        CHECK_NEAR(position[1], 5.0, 1e-12);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, ruckig_trajectory_get_duration(trajectory), position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], 2.0, 1e-7);
        CHECK_NEAR(position[1], 5.0, 1e-12);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

static void test_waypoint_alpha2_fixed_regression_corpus(void) {
    {
        const size_t dofs = 4;
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoints[8] = {
            0.25, -0.15, 0.20, -0.10,
            0.75, -0.45, 0.45, -0.30
        };
        double per_section_minimum_duration[3] = {0.35, 0.60, 0.45};
        double per_section_max_position[12] = {
            0.30, 0.05, 0.25, 0.05,
            0.80, -0.10, 0.50, -0.05,
            1.25, -0.35, 0.80, -0.20
        };
        double per_section_min_position[12] = {
            -0.05, -0.20, -0.05, -0.15,
            0.20, -0.50, 0.15, -0.35,
            0.70, -0.75, 0.40, -0.55
        };
        ruckig_position_extrema_t extrema[4];
        size_t i;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 2), RUCKIG_WORKING);
        for (i = 0; i < dofs; ++i) {
            ruckig_input_max_velocity_data(input)[i] = 1.4;
            ruckig_input_max_acceleration_data(input)[i] = 2.0;
            ruckig_input_max_jerk_data(input)[i] = 4.0;
        }
        ruckig_input_target_position_data(input)[0] = 1.10;
        ruckig_input_target_position_data(input)[1] = -0.65;
        ruckig_input_target_position_data(input)[2] = 0.70;
        ruckig_input_target_position_data(input)[3] = -0.50;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) >= 1.40);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) < 6.00);
        check_waypoint_samples(trajectory, waypoints, 2, dofs);
        CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, dofs), RUCKIG_WORKING);
        CHECK_TRUE(extrema[0].min_position >= -0.05 - 1e-9);
        CHECK_TRUE(extrema[0].max_position <= 1.25 + 1e-9);
        CHECK_TRUE(extrema[1].min_position >= -0.75 - 1e-9);
        CHECK_TRUE(extrema[1].max_position <= 0.05 + 1e-9);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        const size_t dofs = 6;
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoint[6] = {0.20, -0.10, 0.30, -0.20, 5.0, -3.0};
        double position[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        size_t i;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.02, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 1), RUCKIG_WORKING);
        for (i = 0; i < dofs; ++i) {
            ruckig_input_max_velocity_data(input)[i] = 1.5;
            ruckig_input_max_acceleration_data(input)[i] = 2.0;
            ruckig_input_max_jerk_data(input)[i] = 4.0;
        }
        ruckig_input_current_position_data(input)[4] = 5.0;
        ruckig_input_current_position_data(input)[5] = -3.0;
        ruckig_input_target_position_data(input)[0] = 0.50;
        ruckig_input_target_position_data(input)[1] = -0.25;
        ruckig_input_target_position_data(input)[2] = 0.75;
        ruckig_input_target_position_data(input)[3] = -0.45;
        ruckig_input_target_position_data(input)[4] = 5.0;
        ruckig_input_target_position_data(input)[5] = -3.0;
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 4, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 5, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        check_waypoint_samples(trajectory, waypoint, 1, dofs);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, ruckig_trajectory_get_duration(trajectory), position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[4], 5.0, 1e-12);
        CHECK_NEAR(position[5], -3.0, 1e-12);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

static void test_waypoint_041_deep_regression_corpus(void) {
    {
        const size_t dofs = 8;
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoints[24] = {
            0.10, -0.05, 0.12, -0.08, 0.06, -0.04, 7.0, -2.0,
            0.35, -0.20, 0.30, -0.25, 0.18, -0.12, 7.0, -2.0,
            0.70, -0.45, 0.55, -0.40, 0.32, -0.24, 7.0, -2.0
        };
        double position[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        size_t i;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 3), RUCKIG_WORKING);
        for (i = 0; i < dofs; ++i) {
            ruckig_input_max_velocity_data(input)[i] = 1.8;
            ruckig_input_max_acceleration_data(input)[i] = 2.5;
            ruckig_input_max_jerk_data(input)[i] = 5.0;
        }
        ruckig_input_current_position_data(input)[6] = 7.0;
        ruckig_input_current_position_data(input)[7] = -2.0;
        ruckig_input_target_position_data(input)[0] = 1.00;
        ruckig_input_target_position_data(input)[1] = -0.65;
        ruckig_input_target_position_data(input)[2] = 0.75;
        ruckig_input_target_position_data(input)[3] = -0.55;
        ruckig_input_target_position_data(input)[4] = 0.45;
        ruckig_input_target_position_data(input)[5] = -0.33;
        ruckig_input_target_position_data(input)[6] = 7.0;
        ruckig_input_target_position_data(input)[7] = -2.0;
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 6, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 7, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) > 0.0);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) < 8.0);
        check_waypoint_samples(trajectory, waypoints, 3, dofs);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, ruckig_trajectory_get_duration(trajectory), position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], 1.00, 1e-7);
        CHECK_NEAR(position[5], -0.33, 1e-7);
        CHECK_NEAR(position[6], 7.0, 1e-12);
        CHECK_NEAR(position[7], -2.0, 1e-12);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        const size_t dofs = 4;
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoints[8] = {
            0.30, -0.15, 0.22, -0.10,
            0.82, -0.42, 0.46, -0.30
        };
        double per_section_min_velocity[12] = {
            -0.65, -0.75, -0.70, -0.60,
            -0.80, -0.85, -0.78, -0.70,
            -0.90, -0.95, -0.85, -0.80
        };
        double per_section_max_velocity[12] = {
            0.70, 0.75, 0.70, 0.65,
            0.85, 0.90, 0.82, 0.75,
            1.00, 1.05, 0.92, 0.88
        };
        double per_section_min_acceleration[12] = {
            -1.2, -1.2, -1.1, -1.0,
            -1.4, -1.4, -1.3, -1.2,
            -1.6, -1.6, -1.5, -1.4
        };
        double per_section_max_acceleration[12] = {
            1.2, 1.2, 1.1, 1.0,
            1.4, 1.4, 1.3, 1.2,
            1.6, 1.6, 1.5, 1.4
        };
        double per_section_max_jerk[12] = {
            3.0, 3.0, 2.8, 2.6,
            3.4, 3.4, 3.2, 3.0,
            3.8, 3.8, 3.5, 3.3
        };
        double per_section_min_position[12] = {
            -0.05, -0.20, -0.05, -0.15,
            0.25, -0.50, 0.15, -0.35,
            0.75, -0.75, 0.38, -0.55
        };
        double per_section_max_position[12] = {
            0.35, 0.05, 0.25, 0.05,
            0.88, -0.10, 0.50, -0.05,
            1.25, -0.38, 0.78, -0.22
        };
        double per_section_minimum_duration[3] = {0.40, 0.70, 0.50};
        ruckig_position_extrema_t extrema[4];
        size_t i;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 2), RUCKIG_WORKING);
        for (i = 0; i < dofs; ++i) {
            ruckig_input_max_velocity_data(input)[i] = 1.1;
            ruckig_input_max_acceleration_data(input)[i] = 1.8;
            ruckig_input_max_jerk_data(input)[i] = 4.0;
        }
        ruckig_input_target_position_data(input)[0] = 1.15;
        ruckig_input_target_position_data(input)[1] = -0.62;
        ruckig_input_target_position_data(input)[2] = 0.70;
        ruckig_input_target_position_data(input)[3] = -0.48;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) >= 1.60);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) < 7.50);
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
        CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, dofs), RUCKIG_WORKING);
        CHECK_TRUE(extrema[0].min_position >= -0.05 - 1e-9);
        CHECK_TRUE(extrema[0].max_position <= 1.25 + 1e-9);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_t* section_otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_input_t* section_input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        ruckig_trajectory_t* section_trajectory = NULL;
        double waypoints[2] = {0.45, 0.95};
        double durations[2] = {0.0, 0.0};
        double baseline_duration = 0.0;
        double first_time = 0.0;
        bool found = false;
        size_t section_index;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.01, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_create(&section_otg, 1, 0.01), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&section_input, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create(&section_trajectory, 1), RUCKIG_WORKING);
        ruckig_input_current_velocity_data(input)[0] = 0.20;
        ruckig_input_target_position_data(input)[0] = 1.40;
        ruckig_input_target_velocity_data(input)[0] = -0.10;
        ruckig_input_max_velocity_data(input)[0] = 1.2;
        ruckig_input_max_acceleration_data(input)[0] = 2.4;
        ruckig_input_max_jerk_data(input)[0] = 5.0;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, 1), RUCKIG_WORKING);

        ruckig_input_max_velocity_data(section_input)[0] = 1.2;
        ruckig_input_max_acceleration_data(section_input)[0] = 2.4;
        ruckig_input_max_jerk_data(section_input)[0] = 5.0;
        ruckig_input_current_velocity_data(section_input)[0] = 0.20;
        ruckig_input_target_position_data(section_input)[0] = waypoints[0];
        CHECK_EQ_INT(ruckig_calculate(section_otg, section_input, section_trajectory), RUCKIG_WORKING);
        baseline_duration += ruckig_trajectory_get_duration(section_trajectory);
        for (section_index = 1; section_index < 3; ++section_index) {
            ruckig_input_current_position_data(section_input)[0] = waypoints[section_index - 1];
            ruckig_input_current_velocity_data(section_input)[0] = 0.0;
            ruckig_input_target_position_data(section_input)[0] = section_index == 2 ? 1.40 : waypoints[section_index];
            ruckig_input_target_velocity_data(section_input)[0] = section_index == 2 ? -0.10 : 0.0;
            CHECK_EQ_INT(ruckig_calculate(section_otg, section_input, section_trajectory), RUCKIG_WORKING);
            baseline_duration += ruckig_trajectory_get_duration(section_trajectory);
        }

        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) <= baseline_duration + 1e-9);
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 2), RUCKIG_WORKING);
        CHECK_TRUE(durations[0] < durations[1]);
        CHECK_EQ_INT(ruckig_trajectory_get_first_time_at_position(trajectory, 0, waypoints[1], durations[0] + 1e-9, &first_time, &found), RUCKIG_WORKING);
        CHECK_TRUE(found);
        CHECK_NEAR(first_time, durations[1], 1e-7);
        ruckig_trajectory_destroy(section_trajectory);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(section_input);
        ruckig_input_destroy(input);
        ruckig_destroy(section_otg);
        ruckig_destroy(otg);
    }
}

static void test_waypoint_alpha2_quality_regressions(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoints[2] = {0.45, 0.95};
        double durations[2] = {0.0, 0.0};

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.01, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 2), RUCKIG_WORKING);
        ruckig_input_target_position_data(input)[0] = 1.40;
        ruckig_input_current_velocity_data(input)[0] = 0.10;
        ruckig_input_target_velocity_data(input)[0] = -0.05;
        ruckig_input_max_velocity_data(input)[0] = 1.2;
        ruckig_input_max_acceleration_data(input)[0] = 2.4;
        ruckig_input_max_jerk_data(input)[0] = 5.0;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) < 4.50);
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 2), RUCKIG_WORKING);
        CHECK_TRUE(durations[0] < durations[1]);
        CHECK_TRUE(durations[1] < ruckig_trajectory_get_duration(trajectory));
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}


void run_waypoint_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    test_waypoint_validation_and_filter();
    test_waypoint_soft_interruption_update();
    test_waypoint_soft_interruption_alpha2_hardening();
    test_waypoint_fixed_regression_corpus();
    test_waypoint_alpha2_fixed_regression_corpus();
    test_waypoint_041_deep_regression_corpus();
    test_waypoint_alpha2_quality_regressions();
}

void run_waypoint_per_section_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    test_waypoint_alpha2_fixed_regression_corpus();
    test_waypoint_041_deep_regression_corpus();
}

void run_waypoint_quality_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    test_waypoint_fixed_regression_corpus();
    test_waypoint_041_deep_regression_corpus();
    test_waypoint_alpha2_quality_regressions();
}

void run_waypoint_resume_stress_tests(void) {
    test_waypoint_resume_stress_budget_matrix();
    test_waypoint_resume_stress_long_online_loop();
    test_waypoint_resume_stress_allocation_paths();
}

void run_waypoint_resume_quality_tests(void) {
    run_waypoint_resume_quality_audit_tests();
}
