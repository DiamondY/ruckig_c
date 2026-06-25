#include "test_api_internal.h"

static void test_tracking_no_allocation(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    size_t allocations_before;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, 2), RUCKIG_WORKING);
    ruckig_target_state_sequence_position_data(targets)[0] = 0.0;
    ruckig_target_state_sequence_position_data(targets)[1] = 0.005;
    ruckig_target_state_sequence_velocity_data(targets)[0] = 0.5;
    ruckig_target_state_sequence_velocity_data(targets)[1] = 0.5;
    ruckig_target_state_sequence_acceleration_data(targets)[0] = 0.0;
    ruckig_target_state_sequence_acceleration_data(targets)[1] = 0.0;

    ruckig_allocation_counters_reset();
    allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    ruckig_allocation_forbidden_set(false);

    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 8), RUCKIG_WORKING);
    ruckig_allocation_counters_reset();
    allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    {
        ruckig_result_t result = ruckig_tracking_update(tracking, target, input, output);
        CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    }
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    {
        ruckig_result_t result = ruckig_tracking_update_with_lookahead(tracking, targets, input, output);
        CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    }
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    ruckig_allocation_forbidden_set(false);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);

    {
        ruckig_tracking_t* tracking4 = NULL;
        ruckig_target_state_t* target4 = NULL;
        ruckig_target_state_sequence_t* targets4 = NULL;
        ruckig_tracking_output_sequence_t* outputs4 = NULL;
        ruckig_input_t* input4 = NULL;
        ruckig_output_t* output4 = NULL;
        size_t step;
        CHECK_EQ_INT(ruckig_tracking_create(&tracking4, 4, 0.01), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_target_state_create(&target4, 4), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets4, 4, 16), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs4, 4, 16), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&input4, 4), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create(&output4, 4), RUCKIG_WORKING);
        fill_tracking_input_nd(input4, 4);
        CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets4, 16), RUCKIG_WORKING);
        set_tracking_sequence_signal(targets4, 2, 4, 16, 0.01);
        ruckig_allocation_counters_reset();
        allocations_before = ruckig_allocation_count();
        ruckig_allocation_forbidden_set(true);
        for (step = 0; step < 12; ++step) {
            set_tracking_target_signal(target4, 2, 4, (double)step * 0.01);
            CHECK_EQ_INT(ruckig_tracking_update(tracking4, target4, input4, output4), RUCKIG_WORKING);
            CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
            CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
            ruckig_output_pass_to_input(output4, input4);
        }
        CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking4, targets4, input4, outputs4), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
        CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
        ruckig_allocation_forbidden_set(false);
        ruckig_output_destroy(output4);
        ruckig_input_destroy(input4);
        ruckig_tracking_output_sequence_destroy(outputs4);
        ruckig_target_state_sequence_destroy(targets4);
        ruckig_target_state_destroy(target4);
        ruckig_tracking_destroy(tracking4);
    }
}

void run_tracking_no_allocation_tests(void) {
    test_tracking_no_allocation();
}
