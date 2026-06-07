#include <stdio.h>

#include <ruckig_c/ruckig.h>

int main(void) {
    const size_t lookahead_count = 4;
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    size_t step;

    if (ruckig_tracking_create(&tracking, 1, 0.01) != RUCKIG_WORKING
        || ruckig_target_state_sequence_create(&targets, 1, lookahead_count) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_output_create(&output, 1) != RUCKIG_WORKING) {
        return 1;
    }

    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 5.0;
    ruckig_input_min_position_data(input)[0] = -2.5;
    ruckig_input_max_position_data(input)[0] = 2.5;

    if (ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED) != RUCKIG_WORKING
        || ruckig_tracking_set_optimized_strategy(tracking, RUCKIG_TRACKING_OPTIMIZED_BALANCED) != RUCKIG_WORKING
        || ruckig_tracking_set_look_ahead_cycles(tracking, lookahead_count) != RUCKIG_WORKING
        || ruckig_target_state_sequence_set_count(targets, lookahead_count) != RUCKIG_WORKING) {
        return 2;
    }

    for (step = 0; step < 200; ++step) {
        size_t sample;
        for (sample = 0; sample < lookahead_count; ++sample) {
            const double t = (double)(step + sample) * ruckig_tracking_get_delta_time(tracking);
            const size_t offset = sample;
            const double target_position = t < 2.0 ? 0.5 * t : 1.0;
            const double target_velocity = t < 2.0 ? 0.5 : 0.0;
            ruckig_target_state_sequence_position_data(targets)[offset] = target_position;
            ruckig_target_state_sequence_velocity_data(targets)[offset] = target_velocity;
            ruckig_target_state_sequence_acceleration_data(targets)[offset] = 0.0;
        }
        if (ruckig_tracking_update_with_lookahead(tracking, targets, input, output) < 0) {
            return 3;
        }
        ruckig_output_pass_to_input(output, input);
    }

    printf(
        "optimized tracking ramp final position: %.6f status: %d candidates: %zu strategy: %d\n",
        ruckig_output_new_position_data(output)[0],
        (int)ruckig_tracking_get_last_calculation_status(tracking),
        ruckig_tracking_get_last_candidate_count(tracking),
        (int)ruckig_tracking_get_optimized_strategy(tracking)
    );

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
    return 0;
}
