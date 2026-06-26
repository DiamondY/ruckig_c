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

void run_waypoint_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    test_waypoint_validation_and_filter();
    run_waypoint_interrupt_quality_tests();
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
