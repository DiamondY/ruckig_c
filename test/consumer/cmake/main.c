#include <ruckig_c/ruckig.h>

#if defined(RUCKIG_C_EXPECT_STATIC_DEFINE) && !defined(RUCKIG_C_STATIC_DEFINE)
#error "Expected the installed static target to propagate RUCKIG_C_STATIC_DEFINE"
#endif

#if defined(RUCKIG_C_EXPECT_NO_STATIC_DEFINE) && defined(RUCKIG_C_STATIC_DEFINE)
#error "Expected the installed shared target to avoid RUCKIG_C_STATIC_DEFINE"
#endif

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_diagnostics_t diagnostics;
    double position[1] = {0.0};
    int failed = 0;

    failed |= ruckig_create(&otg, 1, 0.01) != RUCKIG_WORKING;
    failed |= ruckig_input_create(&input, 1) != RUCKIG_WORKING;
    failed |= ruckig_trajectory_create(&trajectory, 1) != RUCKIG_WORKING;
    if (!failed) {
        ruckig_input_target_position_data(input)[0] = 1.0;
        ruckig_input_max_velocity_data(input)[0] = 1.0;
        ruckig_input_max_acceleration_data(input)[0] = 1.0;
        ruckig_input_max_jerk_data(input)[0] = 1.0;

        ruckig_diagnostics_init(&diagnostics);
        ruckig_input_max_velocity_data(input)[0] = -1.0;
        failed |= ruckig_validate_input_with_diagnostics(otg, input, true, true, &diagnostics)
            != RUCKIG_ERROR_INVALID_INPUT;
        failed |= diagnostics.result != RUCKIG_ERROR_INVALID_INPUT;
        failed |= diagnostics.scope != RUCKIG_DIAGNOSTIC_SCOPE_INPUT;
        failed |= diagnostics.code != RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT;

        ruckig_input_max_velocity_data(input)[0] = 1.0;
        ruckig_diagnostics_init(&diagnostics);
        failed |= ruckig_calculate_with_diagnostics(otg, input, trajectory, &diagnostics) != RUCKIG_WORKING;
        failed |= diagnostics.result != RUCKIG_WORKING;
        failed |= diagnostics.code != RUCKIG_DIAGNOSTIC_NONE;
        failed |= ruckig_trajectory_at_time(trajectory, 1.0, position, NULL, NULL, NULL, NULL) != RUCKIG_WORKING;
        failed |= position[0] < 0.0 || position[0] > 1.0;
    }

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return failed ? 1 : 0;
}
