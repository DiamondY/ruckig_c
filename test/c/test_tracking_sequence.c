#include "test_api_internal.h"

ruckig_result_t tracking_calculate_sequence_interruptible_under_allocation_guard(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* targets,
    const ruckig_input_t* input,
    ruckig_tracking_output_sequence_t* outputs,
    ruckig_tracking_sequence_continuation_t* continuation
) {
    ruckig_result_t result;
    const size_t allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    result = ruckig_tracking_calculate_sequence_interruptible(tracking, targets, input, outputs, continuation);
    ruckig_allocation_forbidden_set(false);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    return result;
}

ruckig_result_t tracking_resume_sequence_under_allocation_guard(
    ruckig_tracking_t* tracking,
    ruckig_tracking_sequence_continuation_t* continuation,
    ruckig_tracking_output_sequence_t* outputs
) {
    ruckig_result_t result;
    const size_t allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    result = ruckig_tracking_resume_sequence(tracking, continuation, outputs);
    ruckig_allocation_forbidden_set(false);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    return result;
}

static void test_tracking_sequence_fast_continuation_completes_without_budget(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* input = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 4;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_FAST), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 0, 1, count, 0.01);

    CHECK_EQ_INT(
        tracking_calculate_sequence_interruptible_under_allocation_guard(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), count);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_target_count(continuation), count);
    check_tracking_output_sequence(outputs, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, count);
    CHECK_EQ_INT(diagnostics.budget_exhausted_count, 0);

    ruckig_input_destroy(input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_sequence_fast_continuation_resume_budget(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* input = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 4;
    size_t expected_count;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
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
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), 1);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), 1);

    for (expected_count = 2; expected_count <= count; ++expected_count) {
        CHECK_EQ_INT(tracking_resume_sequence_under_allocation_guard(tracking, continuation, outputs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), expected_count);
        CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), expected_count);
        if (expected_count < count) {
            CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));
            CHECK_TRUE(ruckig_tracking_sequence_continuation_was_interrupted(continuation));
            CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_complete(continuation));
        }
    }

    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_complete(continuation));
    check_tracking_output_sequence(outputs, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, count);
    CHECK_EQ_INT(diagnostics.valid_candidate_count, count);
    CHECK_EQ_INT(diagnostics.budget_exhausted_count, count - 1);

    ruckig_tracking_sequence_continuation_reset(continuation);
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), 0);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_target_count(continuation), 0);

    ruckig_input_destroy(input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_sequence_fast_continuation_budget_matrix_and_invalid(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_tracking_t* wrong_tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_output_sequence_t* small_outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* input = NULL;
    const size_t count = 3;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&wrong_tracking, 2, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&small_outputs, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_resume_sequence(tracking, continuation, outputs), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(
        ruckig_tracking_calculate_sequence_interruptible(tracking, targets, input, small_outputs, continuation),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(
        ruckig_tracking_calculate_sequence_interruptible(wrong_tracking, targets, input, outputs, continuation),
        RUCKIG_ERROR_INVALID_INPUT
    );

    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
    CHECK_EQ_INT(
        ruckig_tracking_calculate_sequence_interruptible(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), count);

    ruckig_tracking_sequence_continuation_reset(continuation);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.01), RUCKIG_WORKING);
    ruckig_input_clear_interrupt_calculation_duration(input);
    CHECK_EQ_INT(
        ruckig_tracking_calculate_sequence_interruptible(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), count);

    ruckig_input_destroy(input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(small_outputs);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(wrong_tracking);
    ruckig_tracking_destroy(tracking);
}

void test_tracking_sequence_fast_continuation_delta_time_contract(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_tracking_t* same_dt_tracking = NULL;
    ruckig_tracking_t* wrong_dt_tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* input = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const double delta_time = 0.01;
    const size_t count = 4;
    size_t completed_before;
    size_t output_count_before;
    size_t iteration = 0;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, delta_time), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&same_dt_tracking, 1, delta_time), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&wrong_dt_tracking, 1, 0.02), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_FAST), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 1, 1, count, delta_time);

    CHECK_EQ_INT(
        tracking_calculate_sequence_interruptible_under_allocation_guard(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    completed_before = ruckig_tracking_sequence_continuation_get_completed_count(continuation);
    output_count_before = ruckig_tracking_output_sequence_get_count(outputs);
    CHECK_EQ_INT(completed_before, 1);
    CHECK_EQ_INT(output_count_before, 1);

    CHECK_EQ_INT(
        tracking_resume_sequence_under_allocation_guard(wrong_dt_tracking, continuation, outputs),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(wrong_dt_tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_ERROR);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), completed_before);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), output_count_before);
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(ruckig_tracking_sequence_continuation_was_interrupted(continuation));

    CHECK_EQ_INT(tracking_resume_sequence_under_allocation_guard(same_dt_tracking, continuation, outputs), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_tracking_output_sequence_get_count(outputs) >= output_count_before);
    while (!ruckig_tracking_sequence_continuation_is_complete(continuation) && iteration < 64) {
        CHECK_EQ_INT(tracking_resume_sequence_under_allocation_guard(same_dt_tracking, continuation, outputs), RUCKIG_WORKING);
        ++iteration;
    }
    CHECK_TRUE(iteration < 64);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), count);
    check_tracking_output_sequence(outputs, 1, count, delta_time);

    ruckig_input_destroy(input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(wrong_dt_tracking);
    ruckig_tracking_destroy(same_dt_tracking);
    ruckig_tracking_destroy(tracking);
}

static void check_tracking_output_sequence_matches(
    const ruckig_tracking_output_sequence_t* actual,
    const ruckig_tracking_output_sequence_t* expected,
    size_t dofs,
    size_t count
) {
    size_t step;
    const double* actual_position = ruckig_tracking_output_sequence_new_position_const_data(actual);
    const double* actual_velocity = ruckig_tracking_output_sequence_new_velocity_const_data(actual);
    const double* actual_acceleration = ruckig_tracking_output_sequence_new_acceleration_const_data(actual);
    const double* actual_jerk = ruckig_tracking_output_sequence_new_jerk_const_data(actual);
    const double* actual_time = ruckig_tracking_output_sequence_time_const_data(actual);
    const size_t* actual_section = ruckig_tracking_output_sequence_section_const_data(actual);
    const ruckig_result_t* actual_result = ruckig_tracking_output_sequence_result_const_data(actual);
    const double* expected_position = ruckig_tracking_output_sequence_new_position_const_data(expected);
    const double* expected_velocity = ruckig_tracking_output_sequence_new_velocity_const_data(expected);
    const double* expected_acceleration = ruckig_tracking_output_sequence_new_acceleration_const_data(expected);
    const double* expected_jerk = ruckig_tracking_output_sequence_new_jerk_const_data(expected);
    const double* expected_time = ruckig_tracking_output_sequence_time_const_data(expected);
    const size_t* expected_section = ruckig_tracking_output_sequence_section_const_data(expected);
    const ruckig_result_t* expected_result = ruckig_tracking_output_sequence_result_const_data(expected);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(actual), count);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(expected), count);
    for (step = 0; step < count; ++step) {
        size_t dof;
        CHECK_NEAR(actual_time[step], expected_time[step], 1e-12);
        CHECK_EQ_INT(actual_section[step], expected_section[step]);
        CHECK_EQ_INT(actual_result[step], expected_result[step]);
        for (dof = 0; dof < dofs; ++dof) {
            const size_t offset = step * dofs + dof;
            CHECK_NEAR(actual_position[offset], expected_position[offset], 1e-9);
            CHECK_NEAR(actual_velocity[offset], expected_velocity[offset], 1e-9);
            CHECK_NEAR(actual_acceleration[offset], expected_acceleration[offset], 1e-9);
            CHECK_NEAR(actual_jerk[offset], expected_jerk[offset], 1e-8);
        }
    }
}

static void configure_tracking_sequence_optimized_continuation_strategy(
    ruckig_tracking_t* tracking,
    ruckig_tracking_optimized_strategy_t strategy,
    size_t lookahead_count,
    size_t max_candidates
) {
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, strategy), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, 0.85), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, lookahead_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, max_candidates), RUCKIG_WORKING);
}

void configure_tracking_sequence_optimized_continuation(
    ruckig_tracking_t* tracking,
    size_t lookahead_count,
    size_t max_candidates
) {
    configure_tracking_sequence_optimized_continuation_strategy(
        tracking,
        RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE,
        lookahead_count,
        max_candidates
    );
}

static void test_tracking_sequence_optimized_continuation_large_budget_equivalence(void) {
    ruckig_tracking_t* reference_tracking = NULL;
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* reference_outputs = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* reference_input = NULL;
    ruckig_input_t* input = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 5;

    CHECK_EQ_INT(ruckig_tracking_create(&reference_tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&reference_outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&reference_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(reference_input);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, 0.01);
    configure_tracking_sequence_optimized_continuation(reference_tracking, 4, 16);
    configure_tracking_sequence_optimized_continuation(tracking, 4, 16);

    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(reference_tracking, targets, reference_input, reference_outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(
        tracking_calculate_sequence_interruptible_under_allocation_guard(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), count);
    check_tracking_output_sequence_matches(outputs, reference_outputs, 1, count);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(tracking_optimized_status_is_success(diagnostics.calculation_status));
    CHECK_EQ_INT(diagnostics.mode, RUCKIG_TRACKING_OPTIMIZED);
    CHECK_TRUE(diagnostics.candidate_count >= count);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    ruckig_input_destroy(input);
    ruckig_input_destroy(reference_input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_tracking_output_sequence_destroy(reference_outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
    ruckig_tracking_destroy(reference_tracking);
}

static void test_tracking_sequence_optimized_continuation_resume_equivalence(void) {
    ruckig_tracking_t* reference_tracking = NULL;
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* reference_outputs = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* reference_input = NULL;
    ruckig_input_t* input = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 3;
    size_t iteration = 0;
    size_t last_output_count = 0;

    CHECK_EQ_INT(ruckig_tracking_create(&reference_tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&reference_outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&reference_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(reference_input);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, 0.01);
    configure_tracking_sequence_optimized_continuation(reference_tracking, 3, 8);
    configure_tracking_sequence_optimized_continuation(tracking, 3, 8);

    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(reference_tracking, targets, reference_input, reference_outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(
        tracking_calculate_sequence_interruptible_under_allocation_guard(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    while (!ruckig_tracking_sequence_continuation_is_complete(continuation) && iteration < 256) {
        const size_t output_count = ruckig_tracking_output_sequence_get_count(outputs);
        CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));
        CHECK_TRUE(ruckig_tracking_sequence_continuation_was_interrupted(continuation));
        CHECK_TRUE(output_count >= last_output_count);
        CHECK_EQ_INT(output_count, ruckig_tracking_sequence_continuation_get_completed_count(continuation));
        last_output_count = output_count;
        CHECK_EQ_INT(tracking_resume_sequence_under_allocation_guard(tracking, continuation, outputs), RUCKIG_WORKING);
        ++iteration;
    }
    CHECK_TRUE(iteration > 0);
    CHECK_TRUE(iteration < 256);
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), count);
    check_tracking_output_sequence_matches(outputs, reference_outputs, 1, count);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(tracking_optimized_status_is_success(diagnostics.calculation_status));
    CHECK_TRUE(diagnostics.candidate_count >= count);
    CHECK_TRUE(diagnostics.budget_exhausted_count > 0);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    ruckig_input_destroy(input);
    ruckig_input_destroy(reference_input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_tracking_output_sequence_destroy(reference_outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
    ruckig_tracking_destroy(reference_tracking);
}

static void test_tracking_sequence_optimized_continuation_delta_time_contract(void) {
    ruckig_tracking_t* reference_tracking = NULL;
    ruckig_tracking_t* tracking = NULL;
    ruckig_tracking_t* same_dt_tracking = NULL;
    ruckig_tracking_t* wrong_dt_tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* reference_outputs = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* reference_input = NULL;
    ruckig_input_t* input = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const double delta_time = 0.01;
    const size_t count = 3;
    size_t completed_before;
    size_t output_count_before;
    size_t iteration = 0;

    CHECK_EQ_INT(ruckig_tracking_create(&reference_tracking, 1, delta_time), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, delta_time), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&same_dt_tracking, 1, delta_time), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&wrong_dt_tracking, 1, 0.02), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&reference_outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&reference_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(reference_input);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, delta_time);
    configure_tracking_sequence_optimized_continuation(reference_tracking, 3, 8);
    configure_tracking_sequence_optimized_continuation(tracking, 3, 8);

    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(reference_tracking, targets, reference_input, reference_outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(
        tracking_calculate_sequence_interruptible_under_allocation_guard(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    completed_before = ruckig_tracking_sequence_continuation_get_completed_count(continuation);
    output_count_before = ruckig_tracking_output_sequence_get_count(outputs);

    CHECK_EQ_INT(
        tracking_resume_sequence_under_allocation_guard(wrong_dt_tracking, continuation, outputs),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(wrong_dt_tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_ERROR);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), completed_before);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), output_count_before);
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(ruckig_tracking_sequence_continuation_was_interrupted(continuation));

    while (!ruckig_tracking_sequence_continuation_is_complete(continuation) && iteration < 256) {
        CHECK_EQ_INT(tracking_resume_sequence_under_allocation_guard(same_dt_tracking, continuation, outputs), RUCKIG_WORKING);
        ++iteration;
    }
    CHECK_TRUE(iteration > 0);
    CHECK_TRUE(iteration < 256);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), count);
    check_tracking_output_sequence_matches(outputs, reference_outputs, 1, count);

    ruckig_input_destroy(input);
    ruckig_input_destroy(reference_input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_tracking_output_sequence_destroy(reference_outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(wrong_dt_tracking);
    ruckig_tracking_destroy(same_dt_tracking);
    ruckig_tracking_destroy(tracking);
    ruckig_tracking_destroy(reference_tracking);
}

void run_tracking_sequence_optimized_continuation_equivalence_case(
    ruckig_tracking_optimized_strategy_t strategy,
    size_t dofs,
    size_t count,
    size_t lookahead_count,
    size_t max_candidates,
    bool disable_last_dof,
    double interrupt_duration
) {
    ruckig_tracking_t* reference_tracking = NULL;
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* reference_outputs = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* reference_input = NULL;
    ruckig_input_t* input = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const double delta_time = 0.01;
    size_t iteration = 0;

    CHECK_EQ_INT(ruckig_tracking_create(&reference_tracking, dofs, delta_time), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&tracking, dofs, delta_time), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, dofs, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&reference_outputs, dofs, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, dofs, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, dofs, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&reference_input, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, dofs), RUCKIG_WORKING);

    fill_tracking_input_nd(reference_input, dofs);
    fill_tracking_input_nd(input, dofs);
    if (disable_last_dof && dofs > 1) {
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(reference_input, dofs - 1, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, dofs - 1, false), RUCKIG_WORKING);
    }
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, interrupt_duration), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, dofs, count, delta_time);
    configure_tracking_sequence_optimized_continuation_strategy(reference_tracking, strategy, lookahead_count, max_candidates);
    configure_tracking_sequence_optimized_continuation_strategy(tracking, strategy, lookahead_count, max_candidates);

    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(reference_tracking, targets, reference_input, reference_outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(
        tracking_calculate_sequence_interruptible_under_allocation_guard(tracking, targets, input, outputs, continuation),
        RUCKIG_WORKING
    );
    while (!ruckig_tracking_sequence_continuation_is_complete(continuation) && iteration < 512) {
        CHECK_EQ_INT(tracking_resume_sequence_under_allocation_guard(tracking, continuation, outputs), RUCKIG_WORKING);
        ++iteration;
    }
    CHECK_TRUE(iteration < 512);
    CHECK_TRUE(ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), count);
    check_tracking_output_sequence_matches(outputs, reference_outputs, dofs, count);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(tracking_optimized_status_is_success(diagnostics.calculation_status));
    CHECK_EQ_INT(diagnostics.mode, RUCKIG_TRACKING_OPTIMIZED);
    CHECK_EQ_INT(diagnostics.optimized_strategy, strategy);
    CHECK_EQ_INT(diagnostics.error_step_count, 0);
    CHECK_TRUE(diagnostics.candidate_count >= count);
    if (interrupt_duration == 0.0) {
        CHECK_TRUE(diagnostics.budget_exhausted_count > 0);
    }
    check_tracking_diagnostics_common(tracking, &diagnostics);

    ruckig_input_destroy(input);
    ruckig_input_destroy(reference_input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_tracking_output_sequence_destroy(reference_outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
    ruckig_tracking_destroy(reference_tracking);
}

static void test_tracking_sequence_optimized_continuation_strategy_dof_budget_matrix(void) {
    run_tracking_sequence_optimized_continuation_equivalence_case(
        RUCKIG_TRACKING_OPTIMIZED_STABLE,
        1,
        5,
        3,
        4,
        false,
        1000000000.0
    );
    run_tracking_sequence_optimized_continuation_equivalence_case(
        RUCKIG_TRACKING_OPTIMIZED_BALANCED,
        2,
        4,
        3,
        6,
        true,
        0.0
    );
    run_tracking_sequence_optimized_continuation_equivalence_case(
        RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE,
        4,
        4,
        4,
        16,
        true,
        0.0
    );
    run_tracking_sequence_optimized_continuation_equivalence_case(
        RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE,
        1,
        3,
        2,
        1,
        false,
        0.0
    );
}

static void test_tracking_sequence_optimized_continuation_invalid_inputs(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_tracking_t* wrong_tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_output_sequence_t* small_outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_input_t* input = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 3;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&wrong_tracking, 2, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&small_outputs, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, 0.01);
    configure_tracking_sequence_optimized_continuation(tracking, 3, 8);
    configure_tracking_sequence_optimized_continuation(wrong_tracking, 3, 8);

    CHECK_EQ_INT(
        ruckig_tracking_calculate_sequence_interruptible(tracking, targets, input, small_outputs, continuation),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(
        ruckig_tracking_calculate_sequence_interruptible(wrong_tracking, targets, input, outputs, continuation),
        RUCKIG_ERROR_INVALID_INPUT
    );

    ruckig_target_state_sequence_position_data(targets)[0] = NAN;
    CHECK_EQ_INT(
        ruckig_tracking_calculate_sequence_interruptible(tracking, targets, input, outputs, continuation),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_ERROR);
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_complete(continuation));

    ruckig_input_destroy(input);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(small_outputs);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(wrong_tracking);
    ruckig_tracking_destroy(tracking);
}


void run_tracking_sequence_continuation_api_tests(void) {
    run_tracking_api_lifecycle_tests();
}

void run_tracking_sequence_fast_continuation_tests(void) {
    test_tracking_sequence_fast_continuation_completes_without_budget();
    test_tracking_sequence_fast_continuation_resume_budget();
    test_tracking_sequence_fast_continuation_budget_matrix_and_invalid();
    test_tracking_sequence_fast_continuation_delta_time_contract();
}

void run_tracking_sequence_optimized_continuation_tests(void) {
    test_tracking_sequence_optimized_continuation_large_budget_equivalence();
    test_tracking_sequence_optimized_continuation_resume_equivalence();
    test_tracking_sequence_optimized_continuation_delta_time_contract();
    test_tracking_sequence_optimized_continuation_strategy_dof_budget_matrix();
    test_tracking_sequence_optimized_continuation_invalid_inputs();
}
