#include <math.h>
#include <stdio.h>

#include <ruckig_c/ruckig.h>

static void configure_input(ruckig_input_t* input) {
    ruckig_input_current_position_data(input)[0] = 0.0;
    ruckig_input_current_velocity_data(input)[0] = 0.0;
    ruckig_input_current_acceleration_data(input)[0] = 0.0;
    ruckig_input_target_position_data(input)[0] = 0.0;
    ruckig_input_target_velocity_data(input)[0] = 0.0;
    ruckig_input_target_acceleration_data(input)[0] = 0.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 5.0;
}

int main(void) {
    const size_t count = 3;
    ruckig_tracking_t* tracking = NULL;
    ruckig_input_t* input = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    int exit_code = 1;
    size_t step;
    size_t iteration;

    if (ruckig_tracking_create(&tracking, 1, 0.01) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_target_state_sequence_create(&targets, 1, count) != RUCKIG_WORKING
        || ruckig_tracking_output_sequence_create(&outputs, 1, count) != RUCKIG_WORKING
        || ruckig_tracking_sequence_continuation_create(&continuation, 1, count) != RUCKIG_WORKING) {
        goto cleanup;
    }

    configure_input(input);
    if (ruckig_input_set_interrupt_calculation_duration(input, 0.0) != RUCKIG_WORKING
        || ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED) != RUCKIG_WORKING
        || ruckig_tracking_set_optimized_strategy(tracking, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE) != RUCKIG_WORKING
        || ruckig_tracking_set_look_ahead_cycles(tracking, count) != RUCKIG_WORKING
        || ruckig_tracking_set_max_optimized_candidates(tracking, 8) != RUCKIG_WORKING
        || ruckig_target_state_sequence_set_count(targets, count) != RUCKIG_WORKING) {
        goto cleanup;
    }

    for (step = 0; step < count; ++step) {
        const double t = (double)step * 0.01;
        ruckig_target_state_sequence_position_data(targets)[step] = 0.2 * sin(0.45 * t);
        ruckig_target_state_sequence_velocity_data(targets)[step] = 0.09 * cos(0.45 * t);
        ruckig_target_state_sequence_acceleration_data(targets)[step] = -0.0405 * sin(0.45 * t);
    }

    if (ruckig_tracking_calculate_sequence_interruptible(
            tracking,
            targets,
            input,
            outputs,
            continuation
        ) != RUCKIG_WORKING) {
        goto cleanup;
    }

    for (iteration = 0; iteration < 128 && !ruckig_tracking_sequence_continuation_is_complete(continuation); ++iteration) {
        if (ruckig_tracking_resume_sequence(tracking, continuation, outputs) != RUCKIG_WORKING) {
            goto cleanup;
        }
    }

    if (!ruckig_tracking_sequence_continuation_is_complete(continuation)
        || ruckig_tracking_output_sequence_get_count(outputs) != count) {
        goto cleanup;
    }

    printf(
        "completed %zu/%zu interruptible sequence steps\n",
        ruckig_tracking_sequence_continuation_get_completed_count(continuation),
        ruckig_tracking_sequence_continuation_get_target_count(continuation)
    );

    exit_code = 0;

cleanup:
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_input_destroy(input);
    ruckig_tracking_destroy(tracking);
    return exit_code;
}
