#include <ruckig_c/ruckig.h>

static void configure_input(ruckig_input_t* input) {
    ruckig_input_current_position_data(input)[0] = 0.0;
    ruckig_input_current_velocity_data(input)[0] = 0.0;
    ruckig_input_current_acceleration_data(input)[0] = 0.0;
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_velocity_data(input)[0] = 0.0;
    ruckig_input_target_acceleration_data(input)[0] = 0.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 8.0;
}

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_diagnostics_t diagnostics;
    ruckig_result_t result;
    int exit_code = 1;

    if (ruckig_create(&otg, 1, 0.01) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_output_create(&output, 1) != RUCKIG_WORKING
        || ruckig_trajectory_create(&trajectory, 1) != RUCKIG_WORKING) {
        goto cleanup;
    }

    configure_input(input);
    ruckig_diagnostics_init(&diagnostics);
    result = ruckig_calculate_with_diagnostics(otg, input, trajectory, &diagnostics);
    if (result != RUCKIG_WORKING
        || diagnostics.scope != RUCKIG_DIAGNOSTIC_SCOPE_NONE
        || diagnostics.code != RUCKIG_DIAGNOSTIC_NONE
        || ruckig_trajectory_get_duration(trajectory) <= 0.0) {
        goto cleanup;
    }

    result = ruckig_update(otg, input, output);
    if (result != RUCKIG_WORKING) {
        goto cleanup;
    }
    ruckig_output_pass_to_input(output, input);

    exit_code = 0;

cleanup:
    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return exit_code;
}
