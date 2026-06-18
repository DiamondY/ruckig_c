#include <math.h>

#include <ruckig_c/ruckig.h>

static void configure_basic_input(ruckig_input_t* input) {
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

static int check_no_waypoint_workflow(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_diagnostics_t diagnostics;
    ruckig_result_t result;
    int failed = 1;

    if (ruckig_create(&otg, 1, 0.01) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_output_create(&output, 1) != RUCKIG_WORKING
        || ruckig_trajectory_create(&trajectory, 1) != RUCKIG_WORKING) {
        goto cleanup;
    }

    configure_basic_input(input);
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
    failed = 0;

cleanup:
    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return failed;
}

static int check_waypoint_workflow(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double waypoint[2] = {1.0, -0.5};
    double position[2] = {0.0, 0.0};
    double intermediate_duration[1] = {0.0};
    int failed = 1;

    if (ruckig_create_with_waypoints(&otg, 2, 0.01, 1) != RUCKIG_WORKING
        || ruckig_input_create_with_waypoints(&input, 2, 1) != RUCKIG_WORKING
        || ruckig_trajectory_create_with_waypoints(&trajectory, 2, 1) != RUCKIG_WORKING) {
        goto cleanup;
    }

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_target_position_data(input)[1] = -1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_acceleration_data(input)[1] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 4.0;
    ruckig_input_max_jerk_data(input)[1] = 4.0;
    ruckig_input_min_position_data(input)[0] = -1.0;
    ruckig_input_min_position_data(input)[1] = -2.0;
    ruckig_input_max_position_data(input)[0] = 3.0;
    ruckig_input_max_position_data(input)[1] = 1.0;

    if (ruckig_input_set_intermediate_positions(input, waypoint, 1, 2) != RUCKIG_WORKING
        || ruckig_calculate(otg, input, trajectory) != RUCKIG_WORKING
        || ruckig_trajectory_get_intermediate_duration_count(trajectory) != 1
        || ruckig_trajectory_get_intermediate_durations(trajectory, intermediate_duration, 1) != RUCKIG_WORKING
        || ruckig_trajectory_at_time(trajectory, intermediate_duration[0], position, NULL, NULL, NULL, NULL) != RUCKIG_WORKING
        || fabs(position[0] - waypoint[0]) > 1e-7
        || fabs(position[1] - waypoint[1]) > 1e-7) {
        goto cleanup;
    }
    failed = 0;

cleanup:
    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return failed;
}

static int check_tracking_workflow(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_diagnostics_t diagnostics;
    int failed = 1;

    if (ruckig_tracking_create(&tracking, 1, 0.01) != RUCKIG_WORKING
        || ruckig_target_state_create(&target, 1) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_output_create(&output, 1) != RUCKIG_WORKING) {
        goto cleanup;
    }

    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 8.0;
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
    ruckig_output_pass_to_input(output, input);
    failed = 0;

cleanup:
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
    return failed;
}

int main(void) {
    return check_no_waypoint_workflow()
        || check_waypoint_workflow()
        || check_tracking_workflow();
}
