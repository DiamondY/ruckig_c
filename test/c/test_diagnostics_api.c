#include "test_api_internal.h"

static void configure_public_diagnostics_input(ruckig_input_t* input) {
    ruckig_input_current_position_data(input)[0] = 0.0;
    ruckig_input_current_velocity_data(input)[0] = 0.0;
    ruckig_input_current_acceleration_data(input)[0] = 0.0;
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_velocity_data(input)[0] = 0.0;
    ruckig_input_target_acceleration_data(input)[0] = 0.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.0;
}

static void test_public_diagnostics_init_and_null_parity(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_diagnostics_t diagnostics;
    size_t too_small_size;

    ruckig_diagnostics_init(NULL);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(diagnostics.struct_size, sizeof(ruckig_diagnostics_t));
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_NONE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NONE);

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);
    configure_public_diagnostics_input(input);

    CHECK_EQ_INT(
        ruckig_validate_input_with_diagnostics(otg, input, false, true, NULL),
        ruckig_validate_input(otg, input, false, true)
    );
    CHECK_EQ_INT(
        ruckig_calculate_with_diagnostics(otg, input, trajectory, NULL),
        ruckig_calculate(otg, input, trajectory)
    );
    ruckig_reset(otg);
    CHECK_EQ_INT(
        ruckig_update_with_diagnostics(otg, input, output, NULL),
        ruckig_update(otg, input, output)
    );

    ruckig_diagnostics_init(&diagnostics);
    too_small_size = offsetof(ruckig_diagnostics_t, limit);
    diagnostics.struct_size = too_small_size;
    CHECK_EQ_INT(
        ruckig_validate_input_with_diagnostics(otg, input, false, true, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.struct_size, too_small_size);

    ruckig_diagnostics_init(&diagnostics);
    diagnostics.struct_size = too_small_size;
    diagnostics.result = RUCKIG_FINISHED;
    diagnostics.scope = RUCKIG_DIAGNOSTIC_SCOPE_TRACKING;
    diagnostics.code = RUCKIG_DIAGNOSTIC_UNSUPPORTED;
    CHECK_EQ_INT(
        ruckig_calculate_with_diagnostics(otg, input, trajectory, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.struct_size, too_small_size);
    CHECK_EQ_INT(diagnostics.result, RUCKIG_FINISHED);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_UNSUPPORTED);

    ruckig_diagnostics_init(&diagnostics);
    diagnostics.struct_size = too_small_size;
    diagnostics.result = RUCKIG_FINISHED;
    diagnostics.scope = RUCKIG_DIAGNOSTIC_SCOPE_TRACKING;
    diagnostics.code = RUCKIG_DIAGNOSTIC_UNSUPPORTED;
    CHECK_EQ_INT(
        ruckig_update_with_diagnostics(otg, input, output, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.struct_size, too_small_size);
    CHECK_EQ_INT(diagnostics.result, RUCKIG_FINISHED);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_UNSUPPORTED);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_validate_input_with_diagnostics(otg, input, false, true, &diagnostics),
        RUCKIG_WORKING
    );
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_NONE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NONE);

    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_public_diagnostics_validation_failures(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_input_t* mismatched_input = NULL;
    ruckig_diagnostics_t diagnostics;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&mismatched_input, 2), RUCKIG_WORKING);
    configure_public_diagnostics_input(input);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_validate_input_with_diagnostics(NULL, input, false, true, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.result, RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_INPUT);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NULL_ARGUMENT);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_validate_input_with_diagnostics(otg, mismatched_input, false, true, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_DOF_MISMATCH);
    CHECK_EQ_INT(diagnostics.expected_count, 1);
    CHECK_EQ_INT(diagnostics.actual_count, 2);

    ruckig_input_current_position_data(input)[0] = NAN;
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_validate_input_with_diagnostics(otg, input, true, true, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NONFINITE_VALUE);
    ruckig_input_current_position_data(input)[0] = 0.0;

    ruckig_input_max_velocity_data(input)[0] = -1.0;
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_validate_input_with_diagnostics(otg, input, true, true, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT);
    ruckig_input_max_velocity_data(input)[0] = 1.0;

    ruckig_input_target_velocity_data(input)[0] = 2.0;
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_validate_input_with_diagnostics(otg, input, true, true, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_TARGET_STATE_OUT_OF_LIMITS);
    CHECK_EQ_INT(diagnostics.dof, 0);
    ruckig_input_target_velocity_data(input)[0] = 0.0;

    ruckig_input_current_velocity_data(input)[0] = 2.0;
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_validate_input_with_diagnostics(otg, input, true, true, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_CURRENT_STATE_OUT_OF_LIMITS);
    ruckig_input_current_velocity_data(input)[0] = 0.0;

    input->control_interface = (ruckig_control_interface_t)99;
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_validate_input_with_diagnostics(otg, input, true, true, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_INVALID_ENUM);

    ruckig_input_destroy(mismatched_input);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_public_diagnostics_calculate_failures(void) {
    ruckig_t* otg = NULL;
    ruckig_t* no_waypoint_capacity_otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_input_t* waypoint_input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_trajectory_t* mismatched_trajectory = NULL;
    ruckig_trajectory_t* waypoint_trajectory = NULL;
    ruckig_diagnostics_t diagnostics;
    double waypoint[1] = {0.5};

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&mismatched_trajectory, 2), RUCKIG_WORKING);
    configure_public_diagnostics_input(input);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_calculate_with_diagnostics(NULL, input, trajectory, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_CALCULATION);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NULL_ARGUMENT);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_calculate_with_diagnostics(otg, input, mismatched_trajectory, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_DOF_MISMATCH);

    ruckig_input_max_velocity_data(input)[0] = 0.0;
    ruckig_input_max_acceleration_data(input)[0] = INFINITY;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_calculate_with_diagnostics(otg, input, trajectory, &diagnostics),
        RUCKIG_ERROR_ZERO_LIMITS
    );
    CHECK_EQ_INT(diagnostics.result, RUCKIG_ERROR_ZERO_LIMITS);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_CALCULATION);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_ZERO_LIMIT);
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.0;

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_calculate_with_diagnostics(otg, input, trajectory, &diagnostics),
        RUCKIG_WORKING
    );
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_NONE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NONE);

    CHECK_EQ_INT(ruckig_create(&no_waypoint_capacity_otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&waypoint_input, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&waypoint_trajectory, 1, 1), RUCKIG_WORKING);
    configure_public_diagnostics_input(waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(waypoint_input, waypoint, 1, 1), RUCKIG_WORKING);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_calculate_with_diagnostics(no_waypoint_capacity_otg, waypoint_input, waypoint_trajectory, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_CAPACITY_MISMATCH);
    CHECK_EQ_INT(diagnostics.expected_count, 0);
    CHECK_EQ_INT(diagnostics.actual_count, 1);

    ruckig_trajectory_destroy(waypoint_trajectory);
    ruckig_input_destroy(waypoint_input);
    ruckig_destroy(no_waypoint_capacity_otg);
    ruckig_trajectory_destroy(mismatched_trajectory);
    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_public_diagnostics_update_failures_and_success(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_output_t* mismatched_output = NULL;
    ruckig_diagnostics_t diagnostics;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&mismatched_output, 2), RUCKIG_WORKING);
    configure_public_diagnostics_input(input);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_update_with_diagnostics(NULL, input, output, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_UPDATE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NULL_ARGUMENT);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_update_with_diagnostics(otg, input, mismatched_output, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_DOF_MISMATCH);

    ruckig_input_max_velocity_data(input)[0] = 0.0;
    ruckig_input_max_acceleration_data(input)[0] = INFINITY;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_update_with_diagnostics(otg, input, output, &diagnostics),
        RUCKIG_ERROR_ZERO_LIMITS
    );
    CHECK_EQ_INT(diagnostics.result, RUCKIG_ERROR_ZERO_LIMITS);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_ZERO_LIMIT);
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.0;

    ruckig_reset(otg);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_update_with_diagnostics(otg, input, output, &diagnostics),
        RUCKIG_WORKING
    );
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_NONE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NONE);

    ruckig_output_destroy(mismatched_output);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_public_diagnostics_no_waypoint_interruption(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_diagnostics_t diagnostics;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.05), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    configure_interrupt_boundary_no_waypoint_input(input);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(ruckig_update_with_diagnostics(otg, input, output, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NONE);

    ruckig_output_pass_to_input(output, input);
    ruckig_input_target_position_data(input)[0] += 0.5;
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(ruckig_update_with_diagnostics(otg, input, output, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_UPDATE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_INTERRUPTED);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_public_diagnostics_waypoint_interruption_and_resume(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_diagnostics_t diagnostics;
    size_t too_small_size;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
    configure_alpha2_resume_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(ruckig_update_with_diagnostics(otg, input, output, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_WAYPOINT);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_INTERRUPTED);

    ruckig_output_pass_to_input(output, input);
    CHECK_TRUE(ruckig_waypoint_resume_can_continue(otg, input));

    ruckig_diagnostics_init(&diagnostics);
    too_small_size = offsetof(ruckig_diagnostics_t, limit);
    diagnostics.struct_size = too_small_size;
    CHECK_EQ_INT(
        ruckig_update_with_diagnostics(otg, input, output, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_TRUE(ruckig_waypoint_resume_can_continue(otg, input));

    ruckig_input_target_position_data(input)[0] += 0.04;
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(ruckig_update_with_diagnostics(otg, input, output, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_WAYPOINT);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_RESUME_IDENTITY_MISMATCH);
    CHECK_EQ_INT(diagnostics.expected_count, 2);
    CHECK_EQ_INT(diagnostics.actual_count, 2);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_public_diagnostics_waypoint_resume_mutation_families(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_diagnostics_t diagnostics;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
    configure_alpha2_resume_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update_with_diagnostics(otg, input, output, NULL), RUCKIG_WORKING);
    ruckig_output_pass_to_input(output, input);

    ruckig_input_max_velocity_data(input)[0] = 1.35;
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(ruckig_update_with_diagnostics(otg, input, output, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_WAYPOINT);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_RESUME_IDENTITY_MISMATCH);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
    configure_alpha2_resume_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update_with_diagnostics(otg, input, output, NULL), RUCKIG_WORKING);
    ruckig_output_pass_to_input(output, input);

    input->per_section_minimum_duration[1] = 0.01;
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(ruckig_update_with_diagnostics(otg, input, output, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_WAYPOINT);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_RESUME_IDENTITY_MISMATCH);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
    configure_alpha2_resume_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update_with_diagnostics(otg, input, output, NULL), RUCKIG_WORKING);
    ruckig_output_pass_to_input(output, input);

    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 1, false), RUCKIG_WORKING);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_update_with_diagnostics(otg, input, output, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_WAYPOINT);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_RESUME_IDENTITY_MISMATCH);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}


static void test_tracking_public_diagnostics_getter_contract(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_diagnostics_t diagnostics;
    size_t too_small_size;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, 2), RUCKIG_WORKING);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_tracking_get_last_public_diagnostics(NULL, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.result, RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NULL_ARGUMENT);
    CHECK_EQ_INT(ruckig_tracking_get_last_public_diagnostics(tracking, NULL), RUCKIG_ERROR_INVALID_INPUT);

    ruckig_diagnostics_init(&diagnostics);
    too_small_size = offsetof(ruckig_diagnostics_t, limit);
    diagnostics.struct_size = too_small_size;
    CHECK_EQ_INT(
        ruckig_tracking_get_last_public_diagnostics(tracking, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.struct_size, too_small_size);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(ruckig_tracking_get_last_public_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NONE);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_tracking_sequence_continuation_get_last_diagnostics(NULL, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.result, RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING_SEQUENCE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NULL_ARGUMENT);
    CHECK_EQ_INT(
        ruckig_tracking_sequence_continuation_get_last_diagnostics(continuation, NULL),
        RUCKIG_ERROR_INVALID_INPUT
    );

    ruckig_diagnostics_init(&diagnostics);
    diagnostics.struct_size = too_small_size;
    CHECK_EQ_INT(
        ruckig_tracking_sequence_continuation_get_last_diagnostics(continuation, &diagnostics),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(diagnostics.struct_size, too_small_size);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_tracking_sequence_continuation_get_last_diagnostics(continuation, &diagnostics),
        RUCKIG_WORKING
    );
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING_SEQUENCE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_UNSUPPORTED);
    CHECK_EQ_INT(diagnostics.expected_count, 0);
    CHECK_EQ_INT(diagnostics.actual_count, 0);

    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_public_diagnostics_online_states(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_diagnostics_t diagnostics;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_FAST), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_WORKING);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(ruckig_tracking_get_last_public_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NONE);

    fill_tracking_input_1d(input);
    set_tracking_target_signal(target, 2, 1, 0.02);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 16), RUCKIG_WORKING);
    result = ruckig_tracking_update(tracking, target, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(ruckig_tracking_get_last_public_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NONE);

    fill_tracking_input_1d(input);
    ruckig_target_state_position_data(target)[0] = NAN;
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(ruckig_tracking_get_last_public_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_UNSUPPORTED);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_public_diagnostics_sequence_continuation_states(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_tracking_t* wrong_dt_tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* input = NULL;
    ruckig_diagnostics_t diagnostics;
    const size_t count = 4;
    size_t completed_before;
    size_t iteration = 0;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&wrong_dt_tracking, 1, 0.02), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_FAST), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 1, 1, count, 0.01);

    CHECK_EQ_INT(
        tracking_calculate_sequence_interruptible_under_allocation_guard(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    completed_before = ruckig_tracking_sequence_continuation_get_completed_count(continuation);

    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_tracking_sequence_continuation_get_last_diagnostics(continuation, &diagnostics),
        RUCKIG_WORKING
    );
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING_SEQUENCE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_INTERRUPTED);
    CHECK_EQ_INT(diagnostics.expected_count, count);
    CHECK_EQ_INT(diagnostics.actual_count, completed_before);

    CHECK_EQ_INT(
        tracking_resume_sequence_under_allocation_guard(wrong_dt_tracking, continuation, outputs),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), completed_before);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_tracking_sequence_continuation_get_last_diagnostics(continuation, &diagnostics),
        RUCKIG_WORKING
    );
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_INTERRUPTED);
    CHECK_EQ_INT(diagnostics.actual_count, completed_before);

    while (!ruckig_tracking_sequence_continuation_is_complete(continuation) && iteration < 16) {
        CHECK_EQ_INT(tracking_resume_sequence_under_allocation_guard(tracking, continuation, outputs), RUCKIG_WORKING);
        ++iteration;
    }
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), count);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_tracking_sequence_continuation_get_last_diagnostics(continuation, &diagnostics),
        RUCKIG_WORKING
    );
    CHECK_EQ_INT(diagnostics.result, RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING_SEQUENCE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_NONE);
    CHECK_EQ_INT(diagnostics.expected_count, count);
    CHECK_EQ_INT(diagnostics.actual_count, count);

    ruckig_tracking_sequence_continuation_reset(continuation);
    ruckig_diagnostics_init(&diagnostics);
    CHECK_EQ_INT(
        ruckig_tracking_sequence_continuation_get_last_diagnostics(continuation, &diagnostics),
        RUCKIG_WORKING
    );
    CHECK_EQ_INT(diagnostics.scope, RUCKIG_DIAGNOSTIC_SCOPE_TRACKING_SEQUENCE);
    CHECK_EQ_INT(diagnostics.code, RUCKIG_DIAGNOSTIC_UNSUPPORTED);

    ruckig_input_destroy(input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(wrong_dt_tracking);
    ruckig_tracking_destroy(tracking);
}


void run_public_diagnostics_tests(void) {
    test_public_diagnostics_init_and_null_parity();
    test_public_diagnostics_validation_failures();
    test_public_diagnostics_calculate_failures();
    test_public_diagnostics_update_failures_and_success();
    test_public_diagnostics_no_waypoint_interruption();
    test_public_diagnostics_waypoint_interruption_and_resume();
    test_public_diagnostics_waypoint_resume_mutation_families();
}

void run_tracking_public_diagnostics_tests(void) {
    test_tracking_public_diagnostics_getter_contract();
    test_tracking_public_diagnostics_online_states();
    test_tracking_public_diagnostics_sequence_continuation_states();
}
