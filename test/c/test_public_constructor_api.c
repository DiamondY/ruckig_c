#include "test_api_internal.h"

static void force_waypoint_capacity_overflow_state(ruckig_input_t* input, size_t max_number_of_waypoints) {
    input->max_number_of_waypoints = max_number_of_waypoints;
}

static void force_waypoint_count_overflow_state(ruckig_input_t* input, size_t waypoint_count) {
    input->waypoint_count = waypoint_count;
}

static void force_tracking_sequence_count_overflow_state(
    ruckig_target_state_sequence_t* targets,
    ruckig_tracking_output_sequence_t* outputs,
    ruckig_tracking_sequence_continuation_t* continuation,
    size_t count
) {
    targets->count = count;
    targets->capacity = count;
    outputs->capacity = count;
    continuation->capacity = count;
}

static void force_tracking_output_prefix_overflow_state(
    ruckig_tracking_sequence_continuation_t* continuation,
    size_t count,
    double delta_time
) {
    continuation->target_count = 0;
    continuation->completed_count = 0;
    continuation->active = false;
    continuation->was_interrupted = false;
    continuation->complete = true;
    continuation->mode = RUCKIG_TRACKING_FAST;
    continuation->delta_time = delta_time;
    continuation->output_prefix->count = count;
    continuation->output_prefix->capacity = count;
}

static void force_waypoint_engine_count_overflow_state(ruckig_t* otg, ruckig_input_t* input, size_t waypoint_count) {
    input->waypoint_count = waypoint_count;
    input->max_number_of_waypoints = waypoint_count;
    otg->max_number_of_waypoints = waypoint_count;
    otg->waypoint_engine.phase = RUCKIG_WAYPOINT_ENGINE_PHASE_BASELINE;
}

static void test_constructor_boundary_validation(void) {
    const size_t size_max = (size_t)-1;
    const size_t overflow_waypoints = (size_max / 2u) + 1u;
    double dummy_positions[2] = {0.0, 0.0};
    double threshold[2] = {0.1, 0.1};
    size_t written = size_max;
    bool candidate_evaluated = true;
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_input_t* compare_input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* target_sequence = NULL;
    ruckig_tracking_output_sequence_t* tracking_outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;

    CHECK_EQ_INT(ruckig_create(NULL, 1, 0.01), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_create(&otg, 0, 0.01), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_create(&otg, 1, -0.01), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(otg == NULL);
    CHECK_EQ_INT(ruckig_create(&otg, 1, NAN), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(otg == NULL);
    CHECK_EQ_INT(ruckig_create(&otg, 1, INFINITY), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(otg == NULL);
    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.0), RUCKIG_WORKING);
    CHECK_TRUE(otg != NULL);
    ruckig_destroy(otg);
    otg = NULL;
    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, -0.01, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(otg == NULL);
    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, NAN, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(otg == NULL);
    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, INFINITY, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(otg == NULL);
    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.0, 1), RUCKIG_WORKING);
    CHECK_TRUE(otg != NULL);
    ruckig_destroy(otg);
    otg = NULL;
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(NULL, 1, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 0, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(NULL, 1, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 0, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(NULL, 1, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 0, 1), RUCKIG_ERROR_INVALID_INPUT);

    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, size_max), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(input == NULL);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, size_max), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(trajectory == NULL);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, size_max), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(output == NULL);
    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.01, size_max), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(otg == NULL);

    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 2, overflow_waypoints), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(input == NULL);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 2, overflow_waypoints), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(trajectory == NULL);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 2, overflow_waypoints), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(output == NULL);
    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 2, 0.01, overflow_waypoints), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(otg == NULL);

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 2, 0.01, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&compare_input, 2, 1), RUCKIG_WORKING);
    force_waypoint_capacity_overflow_state(input, overflow_waypoints);
    force_waypoint_capacity_overflow_state(compare_input, overflow_waypoints);
    CHECK_EQ_INT(
        ruckig_input_set_intermediate_positions(input, dummy_positions, overflow_waypoints, 2),
        RUCKIG_ERROR_INVALID_INPUT
    );
    force_waypoint_count_overflow_state(input, overflow_waypoints);
    force_waypoint_count_overflow_state(compare_input, overflow_waypoints);
    CHECK_EQ_INT(
        ruckig_input_get_intermediate_positions(input, dummy_positions, size_max),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(ruckig_input_copy_state(input, compare_input), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(!ruckig_input_equals(input, compare_input));
    CHECK_EQ_INT(
        ruckig_filter_intermediate_positions(otg, input, threshold, 2, dummy_positions, size_max, &written),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(written, 0);
    ruckig_input_destroy(compare_input);
    compare_input = NULL;
    ruckig_input_destroy(input);
    input = NULL;
    ruckig_destroy(otg);
    otg = NULL;

    CHECK_EQ_INT(ruckig_target_state_sequence_create(&target_sequence, 2, overflow_waypoints), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&tracking_outputs, 2, overflow_waypoints), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 2, overflow_waypoints), RUCKIG_ERROR_INVALID_INPUT);

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 2, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&target_sequence, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&tracking_outputs, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 2, 1), RUCKIG_WORKING);
    force_tracking_sequence_count_overflow_state(target_sequence, tracking_outputs, continuation, overflow_waypoints);
    CHECK_EQ_INT(
        ruckig_tracking_calculate_sequence_interruptible(tracking, target_sequence, input, tracking_outputs, continuation),
        RUCKIG_ERROR_INVALID_INPUT
    );
    CHECK_EQ_INT(tracking_outputs->count, 0);
    force_tracking_output_prefix_overflow_state(continuation, overflow_waypoints, 0.01);
    tracking_outputs->count = 1;
    CHECK_EQ_INT(ruckig_tracking_resume_sequence(tracking, continuation, tracking_outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(tracking_outputs->count, 0);
    ruckig_tracking_sequence_continuation_destroy(continuation);
    continuation = NULL;
    ruckig_tracking_output_sequence_destroy(tracking_outputs);
    tracking_outputs = NULL;
    ruckig_target_state_sequence_destroy(target_sequence);
    target_sequence = NULL;
    ruckig_input_destroy(input);
    input = NULL;
    ruckig_tracking_destroy(tracking);
    tracking = NULL;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 2, 0.01, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 2, 1), RUCKIG_WORKING);
    force_waypoint_engine_count_overflow_state(otg, input, overflow_waypoints);
    candidate_evaluated = true;
    CHECK_EQ_INT(ruckig_test_waypoint_engine_step(otg, input, &candidate_evaluated), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(!candidate_evaluated);
    ruckig_input_destroy(input);
    input = NULL;
    ruckig_destroy(otg);
    otg = NULL;

    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(tracking_outputs);
    ruckig_target_state_sequence_destroy(target_sequence);
    ruckig_tracking_destroy(tracking);
    ruckig_input_destroy(compare_input);
    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

void run_constructor_boundary_tests(void) {
    test_constructor_boundary_validation();
}
