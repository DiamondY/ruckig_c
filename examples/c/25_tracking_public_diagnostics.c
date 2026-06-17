#include <stdio.h>

#include <ruckig_c/ruckig.h>

static void configure_input(ruckig_input_t* input) {
    ruckig_input_current_position_data(input)[0] = 0.0;
    ruckig_input_current_velocity_data(input)[0] = 0.0;
    ruckig_input_current_acceleration_data(input)[0] = 0.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 8.0;
}

static void print_public_diagnostics(const char* label, const ruckig_diagnostics_t* diagnostics) {
    printf(
        "%s result=%d scope=%d code=%d dof=%zu value=%.6f limit=%.6f\n",
        label,
        (int)diagnostics->result,
        (int)diagnostics->scope,
        (int)diagnostics->code,
        diagnostics->dof,
        diagnostics->value,
        diagnostics->limit
    );
}

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_diagnostics_t diagnostics;
    int exit_code = 1;

    if (ruckig_create(&otg, 1, 0.01) != RUCKIG_WORKING
        || ruckig_tracking_create(&tracking, 1, 0.01) != RUCKIG_WORKING
        || ruckig_target_state_create(&target, 1) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_output_create(&output, 1) != RUCKIG_WORKING
        || ruckig_tracking_sequence_continuation_create(&continuation, 1, 2) != RUCKIG_WORKING) {
        goto cleanup;
    }

    configure_input(input);
    ruckig_target_state_position_data(target)[0] = 0.05;
    ruckig_target_state_velocity_data(target)[0] = 0.2;
    ruckig_target_state_acceleration_data(target)[0] = 0.0;

    if (ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_FAST) != RUCKIG_WORKING
        || ruckig_tracking_update(tracking, target, input, output) < 0) {
        goto cleanup;
    }

    ruckig_diagnostics_init(&diagnostics);
    if (ruckig_tracking_get_last_public_diagnostics(tracking, &diagnostics) != RUCKIG_WORKING
        || diagnostics.result != RUCKIG_WORKING
        || diagnostics.scope != RUCKIG_DIAGNOSTIC_SCOPE_TRACKING
        || diagnostics.code != RUCKIG_DIAGNOSTIC_NONE) {
        goto cleanup;
    }
    print_public_diagnostics("tracking", &diagnostics);

    ruckig_diagnostics_init(&diagnostics);
    if (ruckig_tracking_sequence_continuation_get_last_diagnostics(continuation, &diagnostics) != RUCKIG_WORKING
        || diagnostics.result != RUCKIG_WORKING
        || diagnostics.scope != RUCKIG_DIAGNOSTIC_SCOPE_TRACKING_SEQUENCE
        || diagnostics.code != RUCKIG_DIAGNOSTIC_UNSUPPORTED) {
        goto cleanup;
    }
    print_public_diagnostics("unstarted-continuation", &diagnostics);

    printf("tracking position %.6f\n", ruckig_output_new_position_data(output)[0]);
    exit_code = 0;

cleanup:
    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
    ruckig_destroy(otg);
    return exit_code;
}
