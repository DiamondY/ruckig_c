#include <stdio.h>
#include <ruckig_c/ruckig.h>

static void print_diagnostics(const char* label, const ruckig_diagnostics_t* diagnostics) {
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

static void configure_valid_input(ruckig_input_t* input) {
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
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_diagnostics_t diagnostics;
    ruckig_result_t result;
    int exit_code = 1;

    if (ruckig_create(&otg, 1, 0.01) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_trajectory_create(&trajectory, 1) != RUCKIG_WORKING) {
        goto cleanup;
    }

    configure_valid_input(input);
    ruckig_input_max_velocity_data(input)[0] = -1.0;
    ruckig_diagnostics_init(&diagnostics);
    result = ruckig_validate_input_with_diagnostics(otg, input, true, true, &diagnostics);
    print_diagnostics("invalid", &diagnostics);
    if (result != RUCKIG_ERROR_INVALID_INPUT
        || diagnostics.code != RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT
        || diagnostics.dof != 0) {
        goto cleanup;
    }

    configure_valid_input(input);
    ruckig_diagnostics_init(&diagnostics);
    result = ruckig_calculate_with_diagnostics(otg, input, trajectory, &diagnostics);
    print_diagnostics("valid", &diagnostics);
    if (result != RUCKIG_WORKING
        || diagnostics.scope != RUCKIG_DIAGNOSTIC_SCOPE_NONE
        || diagnostics.code != RUCKIG_DIAGNOSTIC_NONE) {
        goto cleanup;
    }

    printf("duration %.6f\n", ruckig_trajectory_get_duration(trajectory));
    exit_code = 0;

cleanup:
    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return exit_code;
}
