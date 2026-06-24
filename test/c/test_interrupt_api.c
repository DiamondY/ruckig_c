#include "test_api_internal.h"

void configure_interrupt_boundary_no_waypoint_input(ruckig_input_t* input) {
    ruckig_input_current_position_data(input)[0] = 0.0;
    ruckig_input_current_velocity_data(input)[0] = 0.0;
    ruckig_input_current_acceleration_data(input)[0] = 0.0;
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_velocity_data(input)[0] = 0.0;
    ruckig_input_target_acceleration_data(input)[0] = 0.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 5.0;
}

static void test_interrupt_boundary_waypoint_update_remains_interruptible(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
    configure_soft_interruption_waypoint_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
    CHECK_TRUE(otg->waypoint_engine.active);
    CHECK_TRUE(!otg->waypoint_engine.complete);
    CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) > 0.0);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_interrupt_boundary_no_waypoint_update_ignores_interrupt_and_clears_resume(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* waypoint_input = NULL;
    ruckig_output_t* waypoint_output = NULL;
    ruckig_input_t* no_waypoint_input = NULL;
    ruckig_output_t* no_waypoint_output = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&waypoint_input, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&waypoint_output, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&no_waypoint_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&no_waypoint_output, 1), RUCKIG_WORKING);

    configure_soft_interruption_waypoint_input(waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(waypoint_input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update(otg, waypoint_input, waypoint_output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(waypoint_output));
    CHECK_TRUE(otg->waypoint_engine.active);

    configure_interrupt_boundary_no_waypoint_input(no_waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(no_waypoint_input, 0.0), RUCKIG_WORKING);
    result = ruckig_update(otg, no_waypoint_input, no_waypoint_output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(no_waypoint_output));
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_pass_to_input(no_waypoint_output, no_waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(no_waypoint_input, 1000000000.0), RUCKIG_WORKING);
    result = ruckig_update(otg, no_waypoint_input, no_waypoint_output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(no_waypoint_output));
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_pass_to_input(no_waypoint_output, no_waypoint_input);
    ruckig_input_clear_interrupt_calculation_duration(no_waypoint_input);
    result = ruckig_update(otg, no_waypoint_input, no_waypoint_output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(no_waypoint_output));
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_destroy(no_waypoint_output);
    ruckig_input_destroy(no_waypoint_input);
    ruckig_output_destroy(waypoint_output);
    ruckig_input_destroy(waypoint_input);
    ruckig_destroy(otg);
}

static void test_interrupt_boundary_calculate_clears_waypoint_resume(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 1), RUCKIG_WORKING);
    configure_soft_interruption_waypoint_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(otg->waypoint_engine.active);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) > 0.0);
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_interrupt_boundary_tracking_ignores_interrupt(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 4;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    check_tracking_output_constraints(output, input, 1);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.25), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, 0.01);
    result = ruckig_tracking_update_with_lookahead(tracking, targets, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    check_tracking_output_constraints(output, input, 1);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    fill_tracking_input_1d(input);
    ruckig_input_clear_interrupt_calculation_duration(input);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    check_tracking_output_sequence(outputs, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, count);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_interrupt_boundary_allocation_guard(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* waypoint_input = NULL;
    ruckig_output_t* waypoint_output = NULL;
    ruckig_input_t* no_waypoint_input = NULL;
    ruckig_output_t* no_waypoint_output = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&waypoint_input, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&waypoint_output, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&no_waypoint_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&no_waypoint_output, 1), RUCKIG_WORKING);
    configure_soft_interruption_waypoint_input(waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(waypoint_input, 0.0), RUCKIG_WORKING);

    ruckig_allocation_counters_reset();
    CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, waypoint_input, waypoint_output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(waypoint_output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(waypoint_output));
    CHECK_TRUE(otg->waypoint_engine.active);

    ruckig_output_pass_to_input(waypoint_output, waypoint_input);
    otg->waypoint_engine.best_duration = -1.0;
    CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, waypoint_input, waypoint_output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_new_calculation(waypoint_output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(waypoint_output));
    CHECK_TRUE(otg->waypoint_engine.active);

    configure_interrupt_boundary_no_waypoint_input(no_waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(no_waypoint_input, 0.0), RUCKIG_WORKING);
    result = ruckig_update_under_allocation_guard(otg, no_waypoint_input, no_waypoint_output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(no_waypoint_output));
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_destroy(no_waypoint_output);
    ruckig_input_destroy(no_waypoint_input);
    ruckig_output_destroy(waypoint_output);
    ruckig_input_destroy(waypoint_input);
    ruckig_destroy(otg);
}

static void test_no_waypoint_interrupt_first_solve_publishes_complete_candidate(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.05), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    configure_interrupt_boundary_no_waypoint_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) > 0.0);
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_no_waypoint_interrupt_preserves_incumbent_at_boundary(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result;
    double incumbent_duration;
    double old_time;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.05), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    configure_interrupt_boundary_no_waypoint_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);

    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    incumbent_duration = ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output));
    old_time = ruckig_output_get_time(output);
    CHECK_TRUE(incumbent_duration > old_time);

    ruckig_output_pass_to_input(output, input);
    ruckig_input_target_position_data(input)[0] = 1.8;
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_new_calculation(output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) == incumbent_duration);
    CHECK_TRUE(ruckig_output_get_time(output) > old_time);
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_no_waypoint_interrupt_budget_matrix_and_clear(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result;
    double interrupted_duration;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.05), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    configure_interrupt_boundary_no_waypoint_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);

    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));

    ruckig_output_pass_to_input(output, input);
    ruckig_input_target_position_data(input)[0] = 1.6;
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_new_calculation(output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    interrupted_duration = ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output));

    ruckig_output_pass_to_input(output, input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) != interrupted_duration);

    ruckig_output_pass_to_input(output, input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1.0), RUCKIG_WORKING);
    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_pass_to_input(output, input);
    ruckig_input_clear_interrupt_calculation_duration(input);
    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static ruckig_result_t tracking_update_under_allocation_guard(
    ruckig_tracking_t* tracking,
    ruckig_target_state_t* target,
    ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_result_t result;
    const size_t allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    result = ruckig_tracking_update(tracking, target, input, output);
    ruckig_allocation_forbidden_set(false);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    return result;
}

static ruckig_result_t tracking_update_with_lookahead_under_allocation_guard(
    ruckig_tracking_t* tracking,
    ruckig_target_state_sequence_t* targets,
    ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_result_t result;
    const size_t allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    result = ruckig_tracking_update_with_lookahead(tracking, targets, input, output);
    ruckig_allocation_forbidden_set(false);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    return result;
}

static void test_tracking_interrupt_fast_mode_single_candidate_not_interrupted(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_diagnostics_t diagnostics;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_FAST), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(tracking_update_under_allocation_guard(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    check_tracking_output_constraints(output, input, 1);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    CHECK_EQ_INT(diagnostics.budget_exhausted_count, 0);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_interrupt_optimized_best_so_far_update(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_diagnostics_t diagnostics;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, 4), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 16), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(tracking_update_under_allocation_guard(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    check_tracking_output_constraints(output, input, 1);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    CHECK_EQ_INT(diagnostics.valid_candidate_count, 1);
    CHECK_TRUE(diagnostics.budget_exhausted_count > 0);

    ruckig_output_pass_to_input(output, input);
    fill_tracking_target_ramp(target, 0.01);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
    CHECK_EQ_INT(tracking_update_under_allocation_guard(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(diagnostics.candidate_count > 1);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_interrupt_optimized_lookahead(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 4;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 16), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(tracking_update_with_lookahead_under_allocation_guard(tracking, targets, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    check_tracking_output_constraints(output, input, 1);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    CHECK_EQ_INT(diagnostics.valid_candidate_count, 1);
    CHECK_TRUE(diagnostics.budget_exhausted_count > 0);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_interrupt_sequence_remains_deferred(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 4;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 0, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 16), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    check_tracking_output_sequence(outputs, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(diagnostics.candidate_count >= count);

    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
}

static void test_interrupt_post_release_no_waypoint_quality_corpus(void) {
    const double targets[4] = {0.8, 1.2, 1.7, 2.3};
    size_t case_id;

    for (case_id = 0; case_id < 4; ++case_id) {
        ruckig_t* otg = NULL;
        ruckig_t* reference_otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_trajectory_t* reference = NULL;
        ruckig_result_t result;
        double incumbent_duration = 0.0;
        double reference_duration = 0.0;

        CHECK_EQ_INT(ruckig_create(&otg, 1, 0.02), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_create(&reference_otg, 1, 0.02), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create(&reference, 1), RUCKIG_WORKING);

        configure_interrupt_boundary_no_waypoint_input(input);
        ruckig_input_target_position_data(input)[0] = targets[case_id];
        ruckig_input_max_velocity_data(input)[0] = 0.7 + 0.1 * (double)case_id;
        CHECK_EQ_INT(ruckig_calculate(reference_otg, input, reference), RUCKIG_WORKING);
        reference_duration = ruckig_trajectory_get_duration(reference);
        CHECK_TRUE(reference_duration > 0.0);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        result = ruckig_update_under_allocation_guard(otg, input, output);
        CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), reference_duration, 1e-9);
        CHECK_TRUE(!otg->waypoint_engine.active);

        incumbent_duration = ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output));
        ruckig_output_pass_to_input(output, input);
        ruckig_input_target_position_data(input)[0] = targets[case_id] + 0.45;
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        result = ruckig_update_under_allocation_guard(otg, input, output);
        CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), incumbent_duration, 0.0);
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(reference_otg, input, reference), RUCKIG_WORKING);
        reference_duration = ruckig_trajectory_get_duration(reference);
        CHECK_TRUE(reference_duration > 0.0);
        result = ruckig_update_under_allocation_guard(otg, input, output);
        CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), reference_duration, 1e-9);
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_trajectory_destroy(reference);
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(reference_otg);
        ruckig_destroy(otg);
    }
}

static void test_interrupt_post_release_calculate_and_sequence_boundaries(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* reference = NULL;

        CHECK_EQ_INT(ruckig_create(&otg, 1, 0.01), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create(&reference, 1), RUCKIG_WORKING);
        configure_interrupt_boundary_no_waypoint_input(input);
        ruckig_input_target_position_data(input)[0] = 1.4;
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        CHECK_EQ_INT(ruckig_calculate(otg, input, reference), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(reference) > 0.0);
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_trajectory_destroy(reference);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    test_tracking_interrupt_sequence_remains_deferred();
}


void run_interrupt_boundary_audit_tests(void) {
    test_interrupt_boundary_waypoint_update_remains_interruptible();
    test_interrupt_boundary_no_waypoint_update_ignores_interrupt_and_clears_resume();
    test_interrupt_boundary_calculate_clears_waypoint_resume();
    test_interrupt_boundary_tracking_ignores_interrupt();
    test_interrupt_boundary_allocation_guard();
}

void run_no_waypoint_interrupt_audit_tests(void) {
    test_no_waypoint_interrupt_first_solve_publishes_complete_candidate();
    test_no_waypoint_interrupt_preserves_incumbent_at_boundary();
    test_no_waypoint_interrupt_budget_matrix_and_clear();
}

void run_tracking_interrupt_audit_tests(void) {
    test_tracking_interrupt_fast_mode_single_candidate_not_interrupted();
    test_tracking_interrupt_optimized_best_so_far_update();
    test_tracking_interrupt_optimized_lookahead();
    test_tracking_interrupt_sequence_remains_deferred();
}

void run_interrupt_post_release_quality_tests(void) {
    run_waypoint_resume_quality_audit_tests();
    test_interrupt_post_release_no_waypoint_quality_corpus();
    test_tracking_interrupt_optimized_best_so_far_update();
    test_tracking_interrupt_optimized_lookahead();
    test_interrupt_post_release_calculate_and_sequence_boundaries();
}
