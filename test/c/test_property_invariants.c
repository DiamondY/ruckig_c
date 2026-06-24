#include "test_api_internal.h"

static void test_property_output_and_trajectory_boundaries(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double waypoint[1] = {0.5};
    double durations[1] = {0.0};
    double position[1] = {0.0};
    size_t section = 99;

    CHECK_EQ_INT(ruckig_output_create(NULL, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_output_create(&output, 0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(output == NULL);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(NULL, 1, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 0, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(output == NULL);

    CHECK_TRUE(ruckig_output_new_velocity_data(NULL) == NULL);
    CHECK_TRUE(ruckig_output_new_acceleration_data(NULL) == NULL);
    CHECK_TRUE(ruckig_output_new_jerk_data(NULL) == NULL);
    CHECK_NEAR(ruckig_output_get_time(NULL), 0.0, 0.0);
    CHECK_EQ_INT(ruckig_output_get_new_section(NULL), 0);
    CHECK_TRUE(!ruckig_output_did_section_change(NULL));
    CHECK_TRUE(!ruckig_output_new_calculation(NULL));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(NULL));
    CHECK_NEAR(ruckig_output_get_calculation_duration(NULL), 0.0, 0.0);
    CHECK_TRUE(ruckig_output_get_trajectory(NULL) == NULL);

    CHECK_EQ_INT(ruckig_trajectory_create(NULL, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(trajectory == NULL);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(NULL, 1, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 0, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(trajectory == NULL);
    CHECK_EQ_INT(ruckig_trajectory_get_section_count(NULL), 0);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_duration_count(NULL), 0);

    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_get_section_count(trajectory), 0);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_duration_count(trajectory), 0);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 0), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_trajectory_destroy(trajectory);
    trajectory = NULL;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.01, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 1), RUCKIG_WORKING);
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 4.0;
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_duration_count(trajectory), 1);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, NULL, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 1), RUCKIG_WORKING);
    CHECK_TRUE(durations[0] > 0.0);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, durations[0], position, NULL, NULL, NULL, &section), RUCKIG_WORKING);
    CHECK_EQ_INT(section, 1);
    CHECK_NEAR(position[0], waypoint[0], 1e-7);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_property_no_waypoint_trajectory_invariants(void) {
    ruckig_t* offline_otg = NULL;
    ruckig_t* online_otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[3] = {0.0, 0.0, 0.0};
    double velocity[3] = {0.0, 0.0, 0.0};
    double acceleration[3] = {0.0, 0.0, 0.0};
    double jerk[3] = {0.0, 0.0, 0.0};
    double independent[3] = {0.0, 0.0, 0.0};
    double sample_times[5];
    const double disabled_p0 = 0.60;
    const double disabled_v0 = 0.20;
    const double disabled_a0 = -0.05;
    double duration;
    size_t sample;

    CHECK_TRUE(RUCKIG_RESULT_IS_OK(RUCKIG_WORKING));
    CHECK_TRUE(RUCKIG_RESULT_IS_OK(RUCKIG_FINISHED));
    CHECK_TRUE(!RUCKIG_RESULT_IS_OK(RUCKIG_ERROR));

    CHECK_EQ_INT(ruckig_create(&offline_otg, 3, 0.02), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_create(&online_otg, 3, 0.02), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 3), RUCKIG_WORKING);

    ruckig_input_current_position_data(input)[0] = 0.0;
    ruckig_input_current_position_data(input)[1] = -0.25;
    ruckig_input_current_position_data(input)[2] = disabled_p0;
    ruckig_input_current_velocity_data(input)[0] = 0.0;
    ruckig_input_current_velocity_data(input)[1] = -0.10;
    ruckig_input_current_velocity_data(input)[2] = disabled_v0;
    ruckig_input_current_acceleration_data(input)[0] = 0.0;
    ruckig_input_current_acceleration_data(input)[1] = 0.05;
    ruckig_input_current_acceleration_data(input)[2] = disabled_a0;
    ruckig_input_target_position_data(input)[0] = 1.20;
    ruckig_input_target_position_data(input)[1] = -1.05;
    ruckig_input_target_position_data(input)[2] = 8.0;
    ruckig_input_target_velocity_data(input)[0] = 0.0;
    ruckig_input_target_velocity_data(input)[1] = 0.0;
    ruckig_input_target_velocity_data(input)[2] = 0.0;
    ruckig_input_target_acceleration_data(input)[0] = 0.0;
    ruckig_input_target_acceleration_data(input)[1] = 0.0;
    ruckig_input_target_acceleration_data(input)[2] = 0.0;
    ruckig_input_max_velocity_data(input)[0] = 1.30;
    ruckig_input_max_velocity_data(input)[1] = 1.10;
    ruckig_input_max_velocity_data(input)[2] = 0.10;
    ruckig_input_max_acceleration_data(input)[0] = 2.00;
    ruckig_input_max_acceleration_data(input)[1] = 1.80;
    ruckig_input_max_acceleration_data(input)[2] = 0.10;
    ruckig_input_max_jerk_data(input)[0] = 5.00;
    ruckig_input_max_jerk_data(input)[1] = 4.50;
    ruckig_input_max_jerk_data(input)[2] = 0.10;
    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 2, false), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(offline_otg, input, trajectory), RUCKIG_WORKING);
    duration = ruckig_trajectory_get_duration(trajectory);
    CHECK_TRUE(duration > 0.0);
    CHECK_EQ_INT(ruckig_trajectory_get_independent_min_durations(trajectory, independent, 3), RUCKIG_WORKING);
    CHECK_TRUE(independent[0] > 0.0);
    CHECK_TRUE(independent[1] > 0.0);
    CHECK_NEAR(independent[2], 0.0, 0.0);

    sample_times[0] = 0.0;
    sample_times[1] = duration * 0.25;
    sample_times[2] = duration * 0.50;
    sample_times[3] = duration * 0.75;
    sample_times[4] = duration;
    for (sample = 0; sample < 5; ++sample) {
        size_t dof;
        size_t section = 99u;
        const double t = sample_times[sample];
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, t, position, velocity, acceleration, jerk, &section), RUCKIG_WORKING);
        CHECK_TRUE(section < 8);
        for (dof = 0; dof < 3; ++dof) {
            CHECK_TRUE(isfinite(position[dof]));
            CHECK_TRUE(isfinite(velocity[dof]));
            CHECK_TRUE(isfinite(acceleration[dof]));
            CHECK_TRUE(isfinite(jerk[dof]));
        }
        CHECK_NEAR(position[2], disabled_p0 + disabled_v0 * t + 0.5 * disabled_a0 * t * t, 1e-10);
        CHECK_NEAR(velocity[2], disabled_v0 + disabled_a0 * t, 1e-10);
        CHECK_NEAR(acceleration[2], disabled_a0, 1e-12);
    }
    CHECK_NEAR(position[0], ruckig_input_target_position_const_data(input)[0], 1e-9);
    CHECK_NEAR(position[1], ruckig_input_target_position_const_data(input)[1], 1e-9);
    CHECK_NEAR(velocity[0], 0.0, 1e-9);
    CHECK_NEAR(velocity[1], 0.0, 1e-9);
    CHECK_NEAR(acceleration[0], 0.0, 1e-9);
    CHECK_NEAR(acceleration[1], 0.0, 1e-9);

    CHECK_EQ_INT(ruckig_update(online_otg, input, output), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), duration, 1e-12);
    CHECK_NEAR(ruckig_output_get_time(output), 0.02, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(online_otg);
    ruckig_destroy(offline_otg);
}

static void test_property_tracking_sequence_continuation_partition_equivalence(void) {
    test_tracking_sequence_fast_continuation_delta_time_contract();
    run_tracking_sequence_optimized_continuation_equivalence_case(
        RUCKIG_TRACKING_OPTIMIZED_BALANCED,
        2,
        4,
        3,
        8,
        true,
        0.0
    );
}

static void test_property_waypoint_resume_invariants(void) {
    ruckig_t* otg = NULL;
    ruckig_t* fresh_otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_input_t* fresh_input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* fresh_trajectory = NULL;
    double incumbent_remaining_duration = 0.0;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_create_with_waypoints(&fresh_otg, 4, 0.01, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&fresh_input, 4, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&fresh_trajectory, 4, 3), RUCKIG_WORKING);

    configure_alpha1_resume_stress_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(otg->waypoint_engine.active);
    check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

    incumbent_remaining_duration =
        ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
    ruckig_output_pass_to_input(output, input);
    CHECK_EQ_INT(ruckig_input_copy_state(input, fresh_input), RUCKIG_WORKING);
    ruckig_input_clear_interrupt_calculation_duration(fresh_input);
    CHECK_EQ_INT(ruckig_calculate(fresh_otg, fresh_input, fresh_trajectory), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_trajectory_get_duration(fresh_trajectory) > 0.0);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(!otg->waypoint_engine.active);
    if (ruckig_output_new_calculation(output)) {
        CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
        CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
            < incumbent_remaining_duration - 1e-12);
    }
    check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

    ruckig_trajectory_destroy(fresh_trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(fresh_input);
    ruckig_input_destroy(input);
    ruckig_destroy(fresh_otg);
    ruckig_destroy(otg);
}

static void test_state_machine_input_per_section_boundaries(void) {
    ruckig_input_t* input = NULL;
    double waypoints[4] = {0.25, -0.10, 0.70, 0.15};
    double short_waypoint[2] = {0.35, 0.0};
    double waypoint_readback[4] = {0.0, 0.0, 0.0, 0.0};
    double per_section_values[6] = {1.20, 1.10, 1.15, 1.05, 1.10, 1.00};
    double per_section_negative[6] = {-1.20, -1.10, -1.15, -1.05, -1.10, -1.00};
    double per_section_readback[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double per_section_duration[3] = {0.02, 0.03, 0.01};
    double invalid_negative_duration[3] = {0.02, -0.03, 0.01};
    double invalid_nan_duration[3] = {0.02, NAN, 0.01};
    double duration_readback[3] = {0.0, 0.0, 0.0};

    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 2, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_get_intermediate_positions(input, NULL, 4), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_get_intermediate_positions(input, waypoint_readback, 3), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_get_intermediate_positions(input, waypoint_readback, 4), RUCKIG_WORKING);
    CHECK_NEAR(waypoint_readback[0], waypoints[0], 0.0);
    CHECK_NEAR(waypoint_readback[3], waypoints[3], 0.0);

    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(NULL, per_section_values, 3, 2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, NULL, 3, 2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_values, 2, 2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_values, 3, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_values, 3, 2), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_input_has_per_section_max_velocity(input));
    CHECK_EQ_INT(ruckig_input_get_per_section_max_velocity(input, NULL, 6), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_get_per_section_max_velocity(input, per_section_readback, 5), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_get_per_section_max_velocity(input, per_section_readback, 6), RUCKIG_WORKING);
    CHECK_NEAR(per_section_readback[0], per_section_values[0], 0.0);
    CHECK_NEAR(per_section_readback[5], per_section_values[5], 0.0);
    ruckig_input_clear_per_section_max_velocity(NULL);
    ruckig_input_clear_per_section_max_velocity(input);
    CHECK_TRUE(!ruckig_input_has_per_section_max_velocity(input));
    CHECK_EQ_INT(ruckig_input_get_per_section_max_velocity(input, per_section_readback, 6), RUCKIG_ERROR_INVALID_INPUT);

    CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_negative, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_values, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_negative, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_values, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_values, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_negative, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(NULL, per_section_duration, 3), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, NULL, 3), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_duration, 2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, invalid_negative_duration, 3), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, invalid_nan_duration, 3), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_duration, 3), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_input_has_per_section_minimum_duration(input));
    CHECK_EQ_INT(ruckig_input_get_per_section_minimum_duration(input, NULL, 3), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_get_per_section_minimum_duration(input, duration_readback, 2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_get_per_section_minimum_duration(input, duration_readback, 3), RUCKIG_WORKING);
    CHECK_NEAR(duration_readback[1], per_section_duration[1], 0.0);

    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, short_waypoint, 1, 2), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_input_has_per_section_min_velocity(input));
    CHECK_TRUE(!ruckig_input_has_per_section_max_acceleration(input));
    CHECK_TRUE(!ruckig_input_has_per_section_min_acceleration(input));
    CHECK_TRUE(!ruckig_input_has_per_section_max_jerk(input));
    CHECK_TRUE(!ruckig_input_has_per_section_max_position(input));
    CHECK_TRUE(!ruckig_input_has_per_section_min_position(input));
    CHECK_TRUE(!ruckig_input_has_per_section_minimum_duration(input));
    CHECK_EQ_INT(ruckig_input_get_per_section_min_velocity(input, per_section_readback, 4), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_get_per_section_minimum_duration(input, duration_readback, 2), RUCKIG_ERROR_INVALID_INPUT);

    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, NULL, 0, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_get_intermediate_positions(input, NULL, 0), RUCKIG_WORKING);
    ruckig_input_clear_intermediate_positions(NULL);
    ruckig_input_clear_intermediate_positions(input);
    CHECK_EQ_INT(ruckig_input_get_intermediate_position_count(input), 0);

    ruckig_input_destroy(input);
}

static void configure_state_machine_waypoint_identity_input(ruckig_input_t* input) {
    configure_alpha2_resume_input(input);
}

typedef enum state_machine_waypoint_mutation {
    STATE_MACHINE_WAYPOINT_MUTATE_TARGET,
    STATE_MACHINE_WAYPOINT_MUTATE_WAYPOINTS,
    STATE_MACHINE_WAYPOINT_MUTATE_WAYPOINT_COUNT,
    STATE_MACHINE_WAYPOINT_MUTATE_MAX_LIMIT,
    STATE_MACHINE_WAYPOINT_MUTATE_MIN_LIMIT,
    STATE_MACHINE_WAYPOINT_MUTATE_ENABLED,
    STATE_MACHINE_WAYPOINT_MUTATE_SYNC,
    STATE_MACHINE_WAYPOINT_MUTATE_PER_SECTION_MAX,
    STATE_MACHINE_WAYPOINT_MUTATE_PER_SECTION_MIN_DURATION,
    STATE_MACHINE_WAYPOINT_MUTATE_CLEAR_INTERRUPT
} state_machine_waypoint_mutation_t;

static const char* state_machine_waypoint_mutation_name(state_machine_waypoint_mutation_t mutation) {
    switch (mutation) {
    case STATE_MACHINE_WAYPOINT_MUTATE_TARGET:
        return "target";
    case STATE_MACHINE_WAYPOINT_MUTATE_WAYPOINTS:
        return "waypoints";
    case STATE_MACHINE_WAYPOINT_MUTATE_WAYPOINT_COUNT:
        return "waypoint_count";
    case STATE_MACHINE_WAYPOINT_MUTATE_MAX_LIMIT:
        return "max_limit";
    case STATE_MACHINE_WAYPOINT_MUTATE_MIN_LIMIT:
        return "min_limit";
    case STATE_MACHINE_WAYPOINT_MUTATE_ENABLED:
        return "enabled";
    case STATE_MACHINE_WAYPOINT_MUTATE_SYNC:
        return "sync";
    case STATE_MACHINE_WAYPOINT_MUTATE_PER_SECTION_MAX:
        return "per_section_max";
    case STATE_MACHINE_WAYPOINT_MUTATE_PER_SECTION_MIN_DURATION:
        return "per_section_minimum_duration";
    case STATE_MACHINE_WAYPOINT_MUTATE_CLEAR_INTERRUPT:
        return "clear_interrupt";
    }
    return "unknown";
}

static void apply_state_machine_waypoint_mutation(
    ruckig_input_t* input,
    state_machine_waypoint_mutation_t mutation
) {
    switch (mutation) {
    case STATE_MACHINE_WAYPOINT_MUTATE_TARGET:
        ruckig_input_target_position_data(input)[0] += 0.04;
        break;
    case STATE_MACHINE_WAYPOINT_MUTATE_WAYPOINTS: {
        double waypoints[6] = {
            0.38, -0.12, 0.20,
            0.82, -0.35, 0.48
        };
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, 3), RUCKIG_WORKING);
        break;
    }
    case STATE_MACHINE_WAYPOINT_MUTATE_WAYPOINT_COUNT: {
        double waypoint[3] = {0.50, -0.18, 0.30};
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 3), RUCKIG_WORKING);
        break;
    }
    case STATE_MACHINE_WAYPOINT_MUTATE_MAX_LIMIT:
        ruckig_input_max_velocity_data(input)[0] = 1.35;
        break;
    case STATE_MACHINE_WAYPOINT_MUTATE_MIN_LIMIT:
        ruckig_input_min_position_data(input)[0] = -0.04;
        break;
    case STATE_MACHINE_WAYPOINT_MUTATE_ENABLED:
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 1, false), RUCKIG_WORKING);
        break;
    case STATE_MACHINE_WAYPOINT_MUTATE_SYNC:
        CHECK_EQ_INT(ruckig_input_set_synchronization(input, RUCKIG_SYNCHRONIZATION_NONE), RUCKIG_WORKING);
        break;
    case STATE_MACHINE_WAYPOINT_MUTATE_PER_SECTION_MAX:
        input->per_section_max_velocity[0] = 1.25;
        break;
    case STATE_MACHINE_WAYPOINT_MUTATE_PER_SECTION_MIN_DURATION:
        input->per_section_minimum_duration[1] = 0.01;
        break;
    case STATE_MACHINE_WAYPOINT_MUTATE_CLEAR_INTERRUPT:
        ruckig_input_clear_interrupt_calculation_duration(input);
        break;
    }
}

static void test_state_machine_waypoint_rich_identity_resume(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
    configure_state_machine_waypoint_identity_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(otg->waypoint_engine.active);
    ruckig_output_pass_to_input(output, input);
    CHECK_TRUE(ruckig_waypoint_resume_can_continue(otg, input));

    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(!otg->waypoint_engine.active);
    CHECK_TRUE(otg->waypoint_engine.complete || otg->waypoint_engine.phase == RUCKIG_WAYPOINT_ENGINE_PHASE_IDLE);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void check_state_machine_waypoint_identity_mismatch(state_machine_waypoint_mutation_t mutation) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
    configure_state_machine_waypoint_identity_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(otg->waypoint_engine.active);
    ruckig_output_pass_to_input(output, input);
    CHECK_TRUE(ruckig_waypoint_resume_can_continue(otg, input));

    apply_state_machine_waypoint_mutation(input, mutation);
    if (ruckig_waypoint_resume_can_continue(otg, input)) {
        fprintf(stderr, "waypoint mutation did not invalidate resume identity: %s\n",
                state_machine_waypoint_mutation_name(mutation));
    }
    CHECK_TRUE(!ruckig_waypoint_resume_can_continue(otg, input));
    {
        const ruckig_result_t result = ruckig_update_under_allocation_guard(otg, input, output);
        if (mutation == STATE_MACHINE_WAYPOINT_MUTATE_ENABLED) {
            if (result != RUCKIG_ERROR_INVALID_INPUT) {
                fprintf(stderr, "waypoint mutation update unexpected result for %s: %d\n",
                        state_machine_waypoint_mutation_name(mutation),
                        result);
            }
            CHECK_EQ_INT(result, RUCKIG_ERROR_INVALID_INPUT);
            CHECK_TRUE(!otg->waypoint_engine.active);
            CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        } else {
            if (result != RUCKIG_WORKING) {
                fprintf(stderr, "waypoint mutation update failed for %s: %d\n",
                        state_machine_waypoint_mutation_name(mutation),
                        result);
            }
            CHECK_EQ_INT(result, RUCKIG_WORKING);
        }
    }
    if (mutation == STATE_MACHINE_WAYPOINT_MUTATE_ENABLED) {
        /* Already checked above: disabling a moving DoF is invalid after state handoff. */
    } else if (mutation == STATE_MACHINE_WAYPOINT_MUTATE_CLEAR_INTERRUPT) {
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    } else {
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
    }

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_state_machine_waypoint_identity_mismatches(void) {
    check_state_machine_waypoint_identity_mismatch(STATE_MACHINE_WAYPOINT_MUTATE_TARGET);
    check_state_machine_waypoint_identity_mismatch(STATE_MACHINE_WAYPOINT_MUTATE_WAYPOINTS);
    check_state_machine_waypoint_identity_mismatch(STATE_MACHINE_WAYPOINT_MUTATE_WAYPOINT_COUNT);
    check_state_machine_waypoint_identity_mismatch(STATE_MACHINE_WAYPOINT_MUTATE_MAX_LIMIT);
    check_state_machine_waypoint_identity_mismatch(STATE_MACHINE_WAYPOINT_MUTATE_MIN_LIMIT);
    check_state_machine_waypoint_identity_mismatch(STATE_MACHINE_WAYPOINT_MUTATE_ENABLED);
    check_state_machine_waypoint_identity_mismatch(STATE_MACHINE_WAYPOINT_MUTATE_SYNC);
    check_state_machine_waypoint_identity_mismatch(STATE_MACHINE_WAYPOINT_MUTATE_PER_SECTION_MAX);
    check_state_machine_waypoint_identity_mismatch(STATE_MACHINE_WAYPOINT_MUTATE_PER_SECTION_MIN_DURATION);
    check_state_machine_waypoint_identity_mismatch(STATE_MACHINE_WAYPOINT_MUTATE_CLEAR_INTERRUPT);
}

static void configure_state_machine_waypoint_branch_saturation_input(ruckig_input_t* input) {
    double min_velocity[4] = {-2.0, -2.0, -2.0, -2.0};
    double min_acceleration[4] = {-3.0, -3.0, -3.0, -3.0};
    double waypoints[20] = {
        0.20, -0.10, 0.15, -0.20,
        0.55, -0.30, 0.40, -0.55,
        0.95, -0.55, 0.68, -0.92,
        1.35, -0.82, 0.95, -1.24,
        1.62, -1.05, 1.18, -1.48
    };
    size_t dof;

    for (dof = 0; dof < 4; ++dof) {
        ruckig_input_max_velocity_data(input)[dof] = 2.0;
        ruckig_input_max_acceleration_data(input)[dof] = 3.0;
        ruckig_input_max_jerk_data(input)[dof] = 8.0;
    }
    ruckig_input_target_position_data(input)[0] = 1.90;
    ruckig_input_target_position_data(input)[1] = -1.30;
    ruckig_input_target_position_data(input)[2] = 1.45;
    ruckig_input_target_position_data(input)[3] = -1.78;
    CHECK_EQ_INT(ruckig_input_set_min_velocity(input, min_velocity, 4), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_min_acceleration(input, min_acceleration, 4), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 5, 4), RUCKIG_WORKING);
}

static void test_state_machine_waypoint_branch_queue_saturation(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    bool reached_branch_queue = false;
    size_t iteration;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.02, 5), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 5), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 5), RUCKIG_WORKING);
    configure_state_machine_waypoint_branch_saturation_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    for (iteration = 0; iteration < 256; ++iteration) {
        CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.branch_count <= RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY);
        CHECK_TRUE(otg->waypoint_engine.branch_index <= otg->waypoint_engine.branch_count);
        if (otg->waypoint_engine.branch_queue_valid && otg->waypoint_engine.branch_count > 0) {
            reached_branch_queue = true;
            break;
        }
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        ruckig_output_pass_to_input(output, input);
    }

    CHECK_TRUE(reached_branch_queue);
    CHECK_EQ_INT(otg->waypoint_engine.branch_count, RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY);
    CHECK_TRUE(otg->waypoint_engine.branch_index <= otg->waypoint_engine.branch_count);

    ruckig_output_pass_to_input(output, input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(!otg->waypoint_engine.active);
    CHECK_TRUE(otg->waypoint_engine.complete || otg->waypoint_engine.phase == RUCKIG_WAYPOINT_ENGINE_PHASE_IDLE);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void check_state_machine_tracking_error_diagnostics(ruckig_tracking_t* tracking) {
    ruckig_tracking_diagnostics_t diagnostics;
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_ERROR);
    CHECK_EQ_INT(diagnostics.error_step_count, 1);
    CHECK_EQ_INT(diagnostics.candidate_count, 0);
    check_tracking_diagnostics_common(tracking, &diagnostics);
}

static void resume_tracking_sequence_to_completion(
    ruckig_tracking_t* tracking,
    ruckig_tracking_sequence_continuation_t* continuation,
    ruckig_tracking_output_sequence_t* outputs,
    size_t expected_count
) {
    size_t iteration = 0;
    while (!ruckig_tracking_sequence_continuation_is_complete(continuation) && iteration < 256) {
        CHECK_EQ_INT(tracking_resume_sequence_under_allocation_guard(tracking, continuation, outputs), RUCKIG_WORKING);
        ++iteration;
    }
    CHECK_TRUE(iteration < 256);
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), expected_count);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), expected_count);
}

static void test_state_machine_tracking_empty_continuation_errors(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, 3), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_tracking_resume_sequence(tracking, continuation, outputs), RUCKIG_ERROR_INVALID_INPUT);
    check_state_machine_tracking_error_diagnostics(tracking);

    continuation->delta_time = 0.01;
    continuation->mode = RUCKIG_TRACKING_FAST;
    CHECK_EQ_INT(ruckig_tracking_resume_sequence(tracking, continuation, outputs), RUCKIG_ERROR_INVALID_INPUT);
    check_state_machine_tracking_error_diagnostics(tracking);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), 0);
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(continuation));

    continuation->mode = RUCKIG_TRACKING_OPTIMIZED;
    CHECK_EQ_INT(ruckig_tracking_resume_sequence(tracking, continuation, outputs), RUCKIG_ERROR_INVALID_INPUT);
    check_state_machine_tracking_error_diagnostics(tracking);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), 0);
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(continuation));

    CHECK_EQ_INT(ruckig_tracking_resume_sequence(NULL, continuation, outputs), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_resume_sequence(tracking, NULL, outputs), RUCKIG_ERROR_INVALID_INPUT);
    check_state_machine_tracking_error_diagnostics(tracking);
    CHECK_EQ_INT(ruckig_tracking_resume_sequence(tracking, continuation, NULL), RUCKIG_ERROR_INVALID_INPUT);
    check_state_machine_tracking_error_diagnostics(tracking);

    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_tracking_destroy(tracking);
}

static void test_state_machine_tracking_fast_resume_failure_preserves_state(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_output_sequence_t* small_outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* input = NULL;
    const size_t count = 3;
    size_t completed_before;
    size_t output_count_before;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&small_outputs, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 1, 1, count, 0.01);
    CHECK_EQ_INT(
        tracking_calculate_sequence_interruptible_under_allocation_guard(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));
    completed_before = ruckig_tracking_sequence_continuation_get_completed_count(continuation);
    output_count_before = ruckig_tracking_output_sequence_get_count(outputs);

    CHECK_EQ_INT(
        tracking_resume_sequence_under_allocation_guard(tracking, continuation, small_outputs),
        RUCKIG_ERROR_INVALID_INPUT
    );
    check_state_machine_tracking_error_diagnostics(tracking);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), completed_before);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), output_count_before);
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));

    resume_tracking_sequence_to_completion(tracking, continuation, outputs, count);
    check_tracking_output_sequence(outputs, 1, count, 0.01);

    ruckig_input_destroy(input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(small_outputs);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
}

static void test_state_machine_tracking_optimized_resume_failure_preserves_state(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_output_sequence_t* small_outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* input = NULL;
    const size_t count = 3;
    size_t completed_before;
    size_t output_count_before;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&small_outputs, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, 0.01);
    configure_tracking_sequence_optimized_continuation(tracking, 3, 8);
    CHECK_EQ_INT(
        tracking_calculate_sequence_interruptible_under_allocation_guard(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));
    completed_before = ruckig_tracking_sequence_continuation_get_completed_count(continuation);
    output_count_before = ruckig_tracking_output_sequence_get_count(outputs);

    CHECK_EQ_INT(
        tracking_resume_sequence_under_allocation_guard(tracking, continuation, small_outputs),
        RUCKIG_ERROR_INVALID_INPUT
    );
    check_state_machine_tracking_error_diagnostics(tracking);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), completed_before);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), output_count_before);
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));

    resume_tracking_sequence_to_completion(tracking, continuation, outputs, count);
    check_tracking_output_sequence(outputs, 1, count, 0.01);

    ruckig_input_destroy(input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(small_outputs);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
}


void run_state_machine_branch_coverage_tests(void) {
    test_state_machine_input_per_section_boundaries();
    test_state_machine_waypoint_rich_identity_resume();
    test_state_machine_waypoint_identity_mismatches();
    test_state_machine_waypoint_branch_queue_saturation();
    test_state_machine_tracking_empty_continuation_errors();
    test_state_machine_tracking_fast_resume_failure_preserves_state();
    test_state_machine_tracking_optimized_resume_failure_preserves_state();
}

void run_property_invariant_tests(void) {
    test_property_output_and_trajectory_boundaries();
    test_property_no_waypoint_trajectory_invariants();
    test_property_tracking_sequence_continuation_partition_equivalence();
    test_property_waypoint_resume_invariants();
}
