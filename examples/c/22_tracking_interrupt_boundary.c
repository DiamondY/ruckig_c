#include <stdio.h>

#include <ruckig_c/ruckig.h>

static void configure_input(ruckig_input_t* input) {
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 5.0;
}

static void configure_targets(ruckig_target_state_sequence_t* targets, double delta_time) {
    size_t sample;
    for (sample = 0; sample < 4; ++sample) {
        const double t = (double)sample * delta_time;
        ruckig_target_state_sequence_position_data(targets)[sample] = 0.5 * t;
        ruckig_target_state_sequence_velocity_data(targets)[sample] = 0.5;
        ruckig_target_state_sequence_acceleration_data(targets)[sample] = 0.0;
    }
}

int main(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    ruckig_result_t result;

    if (ruckig_tracking_create(&tracking, 1, 0.01) != RUCKIG_WORKING
        || ruckig_target_state_create(&target, 1) != RUCKIG_WORKING
        || ruckig_target_state_sequence_create(&targets, 1, 4) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_output_create(&output, 1) != RUCKIG_WORKING) {
        return 1;
    }

    configure_input(input);
    ruckig_target_state_position_data(target)[0] = 0.0;
    ruckig_target_state_velocity_data(target)[0] = 0.5;
    ruckig_target_state_acceleration_data(target)[0] = 0.0;
    if (ruckig_target_state_sequence_set_count(targets, 4) != RUCKIG_WORKING
        || ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED) != RUCKIG_WORKING
        || ruckig_tracking_set_optimized_strategy(tracking, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE) != RUCKIG_WORKING
        || ruckig_tracking_set_look_ahead_cycles(tracking, 4) != RUCKIG_WORKING
        || ruckig_tracking_set_max_optimized_candidates(tracking, 16) != RUCKIG_WORKING
        || ruckig_input_set_interrupt_calculation_duration(input, 0.0) != RUCKIG_WORKING) {
        return 2;
    }

    result = ruckig_tracking_update(tracking, target, input, output);
    if (result < 0 || !ruckig_output_was_calculation_interrupted(output)) {
        return 3;
    }

    ruckig_output_pass_to_input(output, input);
    configure_targets(targets, ruckig_tracking_get_delta_time(tracking));
    result = ruckig_tracking_update_with_lookahead(tracking, targets, input, output);
    if (result < 0 || !ruckig_output_was_calculation_interrupted(output)
        || ruckig_tracking_get_last_diagnostics(tracking, &diagnostics) != RUCKIG_WORKING
        || diagnostics.candidate_count == 0
        || diagnostics.budget_exhausted_count == 0) {
        return 4;
    }

    printf(
        "tracking interrupt candidates: %zu status: %d position %.6f\n",
        diagnostics.candidate_count,
        (int)diagnostics.calculation_status,
        ruckig_output_new_position_data(output)[0]
    );

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
    return 0;
}
